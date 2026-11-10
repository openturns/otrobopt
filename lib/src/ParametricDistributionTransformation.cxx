//                                               -*- C++ -*-
/**
 *  @brief Parameter-aware distribution transformation
 *
 *  Copyright 2005-2026 Airbus-EDF-IMACS-ONERA-Phimeca
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
#include "otrobopt/ParametricDistributionTransformation.hxx"

#include <openturns/Description.hxx>
#include <openturns/DistributionTransformation.hxx>
#include <openturns/Exception.hxx>
#include <openturns/Normal.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/ResourceMap.hxx>

#include <algorithm>

using namespace OT;

namespace OTROBOPT
{

namespace
{

Bool isIdentity(const Indices & indices, const UnsignedInteger dimension)
{
  if (indices.getSize() != dimension)
    return false;
  for (UnsignedInteger i = 0; i < dimension; ++i)
    if (indices[i] != i)
      return false;
  return true;
}

Bool startsWith(const String & text, const String & prefix)
{
  return (text.size() >= prefix.size())
         && (text.compare(0, prefix.size(), prefix) == 0);
}

Indices defaultActiveParameter(const Distribution & distribution)
{
  const UnsignedInteger fullDimension = distribution.getParameterDimension();
  if (fullDimension == 0)
    return Indices();
  // There is no generic derivative with respect to the dependence
  // parameters, so they are inactive by default.  The other parameters of
  // composite, elliptical and copula-based distributions are active.
  std::vector<Bool> active(fullDimension, true);
  try
  {
    const Description parameterDescription(distribution.getParameterDescription());
    // The correlation coefficients of the elliptical and normal copula
    // parametrizations are named R_i_j wherever they appear in the full
    // parameter vector.  A mixture of elliptical atoms, for instance,
    // concatenates one correlation block per atom.
    for (UnsignedInteger i = 0; i < fullDimension; ++i)
      if ((i < parameterDescription.getSize())
          && startsWith(parameterDescription[i], "R_"))
        active[i] = false;

    // The other dependence parameters, for instance the parameter of a
    // copula, form the trailing block of the parameter collection.  Remove
    // it only when its description is really the trailing part of the full
    // parameter vector: some distributions, Student for example, prepend
    // their scalar parameters to every block of the collection.
    const Distribution::PointWithDescriptionCollection parameters(distribution.getParametersCollection());
    const UnsignedInteger size = parameters.getSize();
    if ((size > 1) && (parameterDescription.getSize() == fullDimension))
    {
      const PointWithDescription dependence(parameters[size - 1]);
      const UnsignedInteger dependenceDimension = dependence.getDimension();
      if ((dependenceDimension > 0) && (dependenceDimension < fullDimension))
      {
        const Description dependenceDescription(dependence.getDescription());
        Bool isTrailing = (dependenceDescription.getSize() == dependenceDimension);
        for (UnsignedInteger j = 0; isTrailing && (j < dependenceDimension); ++j)
          isTrailing = (dependenceDescription[j]
                        == parameterDescription[fullDimension - dependenceDimension + j]);
        if (isTrailing)
          for (UnsignedInteger j = 0; j < dependenceDimension; ++j)
            active[fullDimension - dependenceDimension + j] = false;
      }
    }
  }
  catch (const Exception &)
  {
    // Keep all parameters active when the distribution does not decompose
    // its parameter vector.
  }
  Indices result(0);
  for (UnsignedInteger i = 0; i < fullDimension; ++i)
    if (active[i])
      result.add(i);
  return result;
}

} /* anonymous namespace */

CLASSNAMEINIT(ParametricDistributionTransformation)

static Factory<ParametricDistributionTransformation> Factory_ParametricDistributionTransformation;

/* Default constructor */
ParametricDistributionTransformation::ParametricDistributionTransformation()
  : FunctionImplementation()
{
  // Nothing to do
}

/* Automatic reference constructor */
ParametricDistributionTransformation::ParametricDistributionTransformation(const Distribution & distribution)
  : FunctionImplementation()
{
  initialize(distribution, Distribution(), false);
}

/* Explicit reference constructor */
ParametricDistributionTransformation::ParametricDistributionTransformation(const Distribution & distribution,
    const Distribution & reference)
  : FunctionImplementation()
{
  initialize(distribution, reference, true);
}

/* Common initialization */
void ParametricDistributionTransformation::initialize(const Distribution & distribution,
    const Distribution & reference,
    const Bool hasReference)
{
  if (distribution.getDimension() == 0)
    throw InvalidArgumentException(HERE) << "The distribution must have a positive dimension";
  if (!distribution.isContinuous())
    throw InvalidArgumentException(HERE) << "Only continuous distributions are supported, got "
                                         << distribution.getImplementation()->getClassName();
  distribution_ = cloneDistribution(distribution);
  configureReference(reference, hasReference);
  activeParameter_ = GetDefaultActiveParameter(distribution_);
  FunctionImplementation::setParameter(getParameter());
  invalidateCache();
}

/* Select the reference distribution */
void ParametricDistributionTransformation::configureReference(const Distribution & reference,
    const Bool hasReference)
{
  const Distribution standardDistribution(distribution_.getStandardDistribution());
  const Bool standardUsable = (standardDistribution.getDimension() == distribution_.getDimension())
                              && (standardDistribution == Normal(distribution_.getDimension()));
  if (hasReference)
  {
    if (reference.getDimension() != distribution_.getDimension())
      throw InvalidDimensionException(HERE) << "The reference dimension ("
                                            << reference.getDimension()
                                            << ") must match the distribution dimension ("
                                            << distribution_.getDimension() << ")";
    if (!reference.isContinuous())
      throw InvalidArgumentException(HERE) << "The reference distribution must be continuous, got "
                                           << reference.getImplementation()->getClassName();
    reference_ = cloneDistribution(reference);
    usesStandardSpace_ = standardUsable && (reference_ == standardDistribution);
  }
  else if (standardUsable)
  {
    reference_ = standardDistribution;
    usesStandardSpace_ = true;
  }
  else
  {
    reference_ = Normal(distribution_.getDimension());
    usesStandardSpace_ = false;
  }
}

/* Copy a distribution without sharing its implementation */
Distribution ParametricDistributionTransformation::cloneDistribution(const Distribution & distribution)
{
  return Distribution(distribution.getImplementation()->clone());
}

/* Virtual constructor */
ParametricDistributionTransformation * ParametricDistributionTransformation::clone() const
{
  return new ParametricDistributionTransformation(*this);
}

/* Invalidate the cached transformation */
void ParametricDistributionTransformation::invalidateCache() const
{
  transformation_ = Function();
  cachedParameter_ = Point();
  cacheValid_ = false;
}

/* Build the transformation for the current parameter */
void ParametricDistributionTransformation::updateTransformation() const
{
  if (cacheValid_ && (cachedParameter_ == getFullParameter()))
    return;
  const Distribution distribution(cloneDistribution(distribution_));
  if (usesStandardSpace_)
    transformation_ = distribution.getInverseIsoProbabilisticTransformation();
  else
    transformation_ = DistributionTransformation(reference_, distribution);
  cachedParameter_ = getFullParameter();
  cacheValid_ = true;
}

/* Evaluation */
Point ParametricDistributionTransformation::operator()(const Point & xi) const
{
  updateTransformation();
  return transformation_(xi);
}

Sample ParametricDistributionTransformation::operator()(const Sample & xi) const
{
  updateTransformation();
  return transformation_(xi);
}

/* Gradient against the reference coordinates */
Matrix ParametricDistributionTransformation::gradient(const Point & xi) const
{
  updateTransformation();
  return transformation_.gradient(xi);
}

/* Gradient against the distribution parameter */
Matrix ParametricDistributionTransformation::parameterGradient(const Point & xi) const
{
  const Point parameter(getParameter());
  const UnsignedInteger parameterDimension = parameter.getDimension();
  const UnsignedInteger outputDimension = getOutputDimension();
  const Scalar epsilon = ResourceMap::GetAsScalar("Evaluation-ParameterEpsilon");
  Matrix gradient(parameterDimension, outputDimension);
  updateTransformation();
  const Point output0(transformation_(xi));
  ParametricDistributionTransformation evaluation(*this);
  for (UnsignedInteger i = 0; i < parameterDimension; ++i)
  {
    Point shifted(parameter);
    shifted[i] += epsilon;
    evaluation.setParameter(shifted);
    const Point output(evaluation(xi));
    for (UnsignedInteger j = 0; j < outputDimension; ++j)
      gradient(i, j) = (output[j] - output0[j]) / epsilon;
  }
  return gradient;
}

/* Distribution accessors */
Distribution ParametricDistributionTransformation::getDistribution() const
{
  return cloneDistribution(distribution_);
}

Distribution ParametricDistributionTransformation::getReferenceDistribution() const
{
  return cloneDistribution(reference_);
}

Bool ParametricDistributionTransformation::usesStandardSpace() const
{
  return usesStandardSpace_;
}

/* Active parameter accessors */
Point ParametricDistributionTransformation::getParameter() const
{
  const Point fullParameter(distribution_.getParameter());
  if (isIdentity(activeParameter_, fullParameter.getDimension()))
    return fullParameter;
  Point parameter(activeParameter_.getSize());
  for (UnsignedInteger i = 0; i < activeParameter_.getSize(); ++i)
    parameter[i] = fullParameter[activeParameter_[i]];
  return parameter;
}

void ParametricDistributionTransformation::setParameter(const Point & parameter)
{
  const UnsignedInteger activeDimension = activeParameter_.getSize();
  if (parameter.getDimension() != activeDimension)
    throw InvalidDimensionException(HERE) << "The parameter dimension ("
                                          << parameter.getDimension()
                                          << ") must match the active parameter dimension ("
                                          << activeDimension << ")";
  if (activeDimension == 0)
    return;
  Point fullParameter(distribution_.getParameter());
  for (UnsignedInteger i = 0; i < activeDimension; ++i)
    fullParameter[activeParameter_[i]] = parameter[i];
  setFullParameter(fullParameter);
}

Description ParametricDistributionTransformation::getParameterDescription() const
{
  const Description fullDescription(distribution_.getParameterDescription());
  const UnsignedInteger fullDimension = distribution_.getParameterDimension();
  if (fullDescription.getSize() != fullDimension)
    return Description::BuildDefault(activeParameter_.getSize(), "s");
  if (isIdentity(activeParameter_, fullDimension))
    return fullDescription;
  Description description(activeParameter_.getSize());
  for (UnsignedInteger i = 0; i < activeParameter_.getSize(); ++i)
    description[i] = fullDescription[activeParameter_[i]];
  return description;
}

UnsignedInteger ParametricDistributionTransformation::getParameterDimension() const
{
  return activeParameter_.getSize();
}

/* Full parameter accessors */
Point ParametricDistributionTransformation::getFullParameter() const
{
  return distribution_.getParameter();
}

void ParametricDistributionTransformation::setFullParameter(const Point & parameter)
{
  const UnsignedInteger fullDimension = distribution_.getParameterDimension();
  if (parameter.getDimension() != fullDimension)
    throw InvalidDimensionException(HERE) << "The full parameter dimension ("
                                          << parameter.getDimension()
                                          << ") must match the distribution parameter dimension ("
                                          << fullDimension << ")";
  Distribution candidate(cloneDistribution(distribution_));
  candidate.setParameter(parameter);
  distribution_ = candidate;
  FunctionImplementation::setParameter(getParameter());
  invalidateCache();
}

Description ParametricDistributionTransformation::getFullParameterDescription() const
{
  return distribution_.getParameterDescription();
}

/* Active parameter indices */
Indices ParametricDistributionTransformation::getActiveParameter() const
{
  return activeParameter_;
}

void ParametricDistributionTransformation::setActiveParameter(const Indices & active)
{
  Indices sorted(active);
  std::sort(sorted.begin(), sorted.end());
  const auto last = std::unique(sorted.begin(), sorted.end());
  if (last != sorted.end())
    throw InvalidArgumentException(HERE) << "Active parameter indices must be unique";
  const UnsignedInteger fullDimension = distribution_.getParameterDimension();
  for (UnsignedInteger i = 0; i < active.getSize(); ++i)
    if (active[i] >= fullDimension)
      throw InvalidArgumentException(HERE) << "Active parameter index " << active[i]
                                           << " is out of range [0, " << fullDimension << ")";
  activeParameter_ = active;
  FunctionImplementation::setParameter(getParameter());
}

/* Default active parameter indices for a distribution */
Indices ParametricDistributionTransformation::GetDefaultActiveParameter(const Distribution & distribution)
{
  return defaultActiveParameter(distribution);
}

/* Dimension accessors */
UnsignedInteger ParametricDistributionTransformation::getInputDimension() const
{
  return reference_.getDimension();
}

UnsignedInteger ParametricDistributionTransformation::getOutputDimension() const
{
  return distribution_.getDimension();
}

Description ParametricDistributionTransformation::getInputDescription() const
{
  const Description description(reference_.getDescription());
  if (description.getSize() == getInputDimension())
    return description;
  return Description::BuildDefault(getInputDimension(), "xi");
}

Description ParametricDistributionTransformation::getOutputDescription() const
{
  const Description description(distribution_.getDescription());
  if (description.getSize() == getOutputDimension())
    return description;
  return Description::BuildDefault(getOutputDimension(), "theta");
}

/* Parallel accessor */
Bool ParametricDistributionTransformation::isParallel() const
{
  return false;
}

/* String converter */
String ParametricDistributionTransformation::__repr__() const
{
  OSS oss;
  oss << "class=" << ParametricDistributionTransformation::GetClassName()
      << " distribution=" << distribution_
      << " reference=" << reference_
      << " usesStandardSpace=" << usesStandardSpace_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void ParametricDistributionTransformation::save(Advocate & adv) const
{
  FunctionImplementation::save(adv);
  adv.saveAttribute("distribution_", distribution_);
  adv.saveAttribute("reference_", reference_);
  adv.saveAttribute("usesStandardSpace_", usesStandardSpace_);
  adv.saveAttribute("activeParameter_", activeParameter_);
}

/* Method load() reloads the object from the StorageManager */
void ParametricDistributionTransformation::load(Advocate & adv)
{
  FunctionImplementation::load(adv);
  adv.loadAttribute("distribution_", distribution_);
  adv.loadAttribute("reference_", reference_);
  adv.loadAttribute("usesStandardSpace_", usesStandardSpace_);
  if (adv.hasAttribute("activeParameter_"))
    adv.loadAttribute("activeParameter_", activeParameter_);
  else
    activeParameter_ = GetDefaultActiveParameter(distribution_);
  const UnsignedInteger fullDimension = distribution_.getParameterDimension();
  Bool valid = true;
  for (UnsignedInteger i = 0; i < activeParameter_.getSize(); ++i)
    if (activeParameter_[i] >= fullDimension)
      valid = false;
  if (!valid)
    activeParameter_ = GetDefaultActiveParameter(distribution_);
  FunctionImplementation::setParameter(getParameter());
  invalidateCache();
}

} /* namespace OTROBOPT */
