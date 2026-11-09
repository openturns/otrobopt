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

#include <openturns/DistributionTransformation.hxx>
#include <openturns/Exception.hxx>
#include <openturns/Normal.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/ResourceMap.hxx>

using namespace OT;

namespace OTROBOPT
{

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
  FunctionImplementation::setParameter(distribution_.getParameter());
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
  if (cacheValid_ && (cachedParameter_ == getParameter()))
    return;
  const Distribution distribution(cloneDistribution(distribution_));
  if (usesStandardSpace_)
    transformation_ = distribution.getInverseIsoProbabilisticTransformation();
  else
    transformation_ = DistributionTransformation(reference_, distribution);
  cachedParameter_ = getParameter();
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

/* Parameter accessors */
Point ParametricDistributionTransformation::getParameter() const
{
  return distribution_.getParameter();
}

void ParametricDistributionTransformation::setParameter(const Point & parameter)
{
  const UnsignedInteger parameterDimension = distribution_.getParameterDimension();
  if (parameter.getDimension() != parameterDimension)
    throw InvalidDimensionException(HERE) << "The parameter dimension ("
                                          << parameter.getDimension()
                                          << ") must match the distribution parameter dimension ("
                                          << parameterDimension << ")";
  Distribution candidate(cloneDistribution(distribution_));
  candidate.setParameter(parameter);
  distribution_ = candidate;
  FunctionImplementation::setParameter(distribution_.getParameter());
  invalidateCache();
}

Description ParametricDistributionTransformation::getParameterDescription() const
{
  return distribution_.getParameterDescription();
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
}

/* Method load() reloads the object from the StorageManager */
void ParametricDistributionTransformation::load(Advocate & adv)
{
  FunctionImplementation::load(adv);
  adv.loadAttribute("distribution_", distribution_);
  adv.loadAttribute("reference_", reference_);
  adv.loadAttribute("usesStandardSpace_", usesStandardSpace_);
  FunctionImplementation::setParameter(distribution_.getParameter());
  invalidateCache();
}

} /* namespace OTROBOPT */
