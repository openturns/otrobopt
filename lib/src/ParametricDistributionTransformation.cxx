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
#include <openturns/EllipticalDistribution.hxx>
#include <openturns/Exception.hxx>
#include <openturns/Normal.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/ResourceMap.hxx>
#include <openturns/TriangularMatrix.hxx>

#include <algorithm>
#include <cmath>

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

Indices defaultActiveParameter(const Distribution & distribution)
{
  const UnsignedInteger fullDimension = distribution.getParameterDimension();
  Indices active(fullDimension);
  active.fill();
  if (fullDimension == 0)
    return active;
  try
  {
    // For composite and elliptical distributions, the last element of the
    // parameter collection is the dependence or copula block.  There is no
    // generic derivative with respect to these parameters, so they are
    // inactive by default.  Distributions that expose a single compact block
    // keep all their parameters active.
    const Distribution::PointWithDescriptionCollection parameters(distribution.getParametersCollection());
    const UnsignedInteger size = parameters.getSize();
    if (size > 1)
    {
      const UnsignedInteger dependenceDimension = parameters[size - 1].getDimension();
      if ((dependenceDimension > 0) && (dependenceDimension < fullDimension))
      {
        active = Indices(fullDimension - dependenceDimension);
        active.fill();
      }
    }
  }
  catch (const Exception &)
  {
    // Keep all parameters active when the distribution does not decompose
    // its parameter vector.
  }
  return active;
}

/* Check the documented parameter layout of an OpenTURNS Normal distribution:
   [mu_0, sigma_0, ..., mu_{d-1}, sigma_{d-1}, R_1_0, R_2_0, R_2_1, ...]. */
Bool isNormalParameterLayout(const Description & description,
                             const UnsignedInteger dimension)
{
  const UnsignedInteger expectedDimension = 2 * dimension + dimension * (dimension - 1) / 2;
  if (description.getSize() != expectedDimension)
    return false;
  for (UnsignedInteger i = 0; i < dimension; ++i)
  {
    OSS ossMean;
    ossMean << "mu_" << i;
    OSS ossSigma;
    ossSigma << "sigma_" << i;
    if (description[2 * i] != ossMean.str())
      return false;
    if (description[2 * i + 1] != ossSigma.str())
      return false;
  }
  UnsignedInteger index = 2 * dimension;
  for (UnsignedInteger i = 1; i < dimension; ++i)
    for (UnsignedInteger j = 0; j < i; ++j)
    {
      OSS oss;
      oss << "R_" << i << "_" << j;
      if (description[index] != oss.str())
        return false;
      ++index;
    }
  return true;
}

/* Derivative of a lower triangular Cholesky factor A of C = A A^T with
   respect to a parameter, given the derivative dC of C:
   dA = A Phi(A^{-1} dC A^{-T}), Phi keeping the lower triangle and halving
   the diagonal. */
Matrix choleskyGradient(const Matrix & cholesky,
                        const Matrix & covarianceGradient)
{
  const UnsignedInteger dimension = cholesky.getNbRows();
  // P = A^{-1} dC, then M = P A^{-T} = (A^{-1} P^T)^T
  const Matrix left(cholesky.solveLinearSystem(covarianceGradient));
  const Matrix middle(cholesky.solveLinearSystem(left.transpose()).transpose());
  Matrix lower(middle);
  for (UnsignedInteger i = 0; i < dimension; ++i)
  {
    for (UnsignedInteger j = i + 1; j < dimension; ++j)
      lower(i, j) = 0.0;
    lower(i, i) *= 0.5;
  }
  return cholesky * lower;
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
  invalidateCache();
  refreshEvaluationDelegates();
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

/* Keep the composition delegates in sync with the current transformation */
void ParametricDistributionTransformation::refreshEvaluationDelegates()
{
  updateTransformation();
  setEvaluation(transformation_.getEvaluation());
  setGradient(transformation_.getGradient());
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
  updateTransformation();
  const Point x(transformation_(xi));
  const UnsignedInteger outputDimension = x.getDimension();
  const Indices active(getActiveParameter());
  const UnsignedInteger parameterDimension = active.getSize();
  const Point fullParameter(getFullParameter());
  Distribution distribution(cloneDistribution(distribution_));
  distribution.setParameter(fullParameter);

  // Normal distribution in standard space: T_s(xi) = mu + A xi, where A is
  // the lower triangular Cholesky factor of the covariance matrix.  The
  // parameter vector is [mu_i, sigma_i, R_1_0, ...], so every parameter
  // derivative is either a unit vector (location) or dA xi.
  if (usesStandardSpace_)
  {
    const Normal * p_normal = dynamic_cast<const Normal *>(distribution.getImplementation().get());
    if ((p_normal != nullptr)
        && isNormalParameterLayout(p_normal->getParameterDescription(), outputDimension))
    {
      const Point sigma(p_normal->getSigma());
      Bool positiveScale = true;
      for (UnsignedInteger i = 0; i < outputDimension; ++i)
        positiveScale = positiveScale && (sigma[i] > 0.0);
      if (positiveScale)
      {
        const TriangularMatrix triangularCholesky(p_normal->getCholesky());
        Matrix cholesky(outputDimension, outputDimension);
        for (UnsignedInteger i = 0; i < outputDimension; ++i)
          for (UnsignedInteger j = 0; j <= i; ++j)
            cholesky(i, j) = triangularCholesky(i, j);
        const CorrelationMatrix correlation(p_normal->getR());
        Matrix gradient(parameterDimension, outputDimension);
        for (UnsignedInteger i = 0; i < parameterDimension; ++i)
        {
          const UnsignedInteger index = active[i];
          // A location parameter shifts the whole output
          if ((index < 2 * outputDimension) && (index % 2 == 0))
          {
            gradient(i, index / 2) = 1.0;
            continue;
          }
          Matrix covarianceGradient(outputDimension, outputDimension);
          if (index < 2 * outputDimension)
          {
            // A scale parameter: dC_kk = 2 sigma_k and
            // dC_kj = sigma_j R_kj for j != k
            const UnsignedInteger k = (index - 1) / 2;
            covarianceGradient(k, k) = 2.0 * sigma[k];
            for (UnsignedInteger j = 0; j < outputDimension; ++j)
              if (j != k)
                covarianceGradient(k, j) = covarianceGradient(j, k) = sigma[j] * correlation(k, j);
          }
          else
          {
            // A correlation parameter, in the lower triangular order
            UnsignedInteger position = 2 * outputDimension;
            UnsignedInteger row = 1;
            UnsignedInteger column = 0;
            while (position != index)
            {
              ++column;
              if (column == row)
              {
                ++row;
                column = 0;
              }
              ++position;
            } // while
            covarianceGradient(row, column) = covarianceGradient(column, row) = sigma[row] * sigma[column];
          }
          const Point derivative(choleskyGradient(cholesky, covarianceGradient) * xi);
          for (UnsignedInteger j = 0; j < outputDimension; ++j)
            gradient(i, j) = derivative[j];
        } // for i
        return gradient;
      } // positiveScale
    } // layout
  } // standard space

  // One-dimensional distribution with the standard normal reference: the
  // transformation is the inverse iso-probabilistic transformation, i.e.
  // T_s(xi) = F_s^{-1}(Phi(xi)), so F_s(T_s(xi)) = Phi(xi) with a reference
  // that does not depend on s.  The implicit function theorem then gives
  //   dT/ds_i = -(dF_s/ds_i)(T_s(xi)) / p_s(T_s(xi)).
  // The CDF gradient of the distribution carries the moving-support term, so
  // this is exact for location-scale families and for bounded supports alike.
  // The standard branch is required: for a Student distribution the standard
  // distribution is Student(nu, 0, 1), which depends on nu, so its inverse
  // iso-probabilistic transformation is not a quantile map and the identity
  // above does not hold.  Such families fall back to finite differences.
  if (usesStandardSpace_ && (outputDimension == 1) && distribution.isContinuous())
  {
    const Scalar pdf = distribution.computePDF(x);
    if (std::isfinite(pdf) && (pdf > 0.0))
    {
      const Point cdfGradient(distribution.computeCDFGradient(x));
      if (cdfGradient.getDimension() == fullParameter.getDimension())
      {
        Matrix gradient(parameterDimension, 1);
        for (UnsignedInteger i = 0; i < parameterDimension; ++i)
          gradient(i, 0) = -cdfGradient[active[i]] / pdf;
        return gradient;
      }
    }
  }

  // Other distributions and non-standard references: forward finite
  // differences over the parameter, with a backward step when the forward
  // step leaves the parameter domain.
  const Scalar epsilon = ResourceMap::GetAsScalar("Evaluation-ParameterEpsilon");
  Matrix gradient(parameterDimension, outputDimension);
  const Point output0(transformation_(xi));
  ParametricDistributionTransformation evaluation(*this);
  for (UnsignedInteger i = 0; i < parameterDimension; ++i)
  {
    Point shifted(fullParameter);
    shifted[active[i]] += epsilon;
    Scalar step = epsilon;
    Point output;
    try
    {
      evaluation.setFullParameter(shifted);
      output = evaluation(xi);
    }
    catch (const Exception &)
    {
      // The forward step leaves the parameter domain (for example the upper
      // bound of a Uniform distribution): use a backward difference instead.
      step = -epsilon;
      shifted = fullParameter;
      shifted[active[i]] += step;
      evaluation.setFullParameter(shifted);
      output = evaluation(xi);
    }
    for (UnsignedInteger j = 0; j < outputDimension; ++j)
      gradient(i, j) = (output[j] - output0[j]) / step;
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
  // Repeating the current parameter must not rebuild the cached
  // transformation: the measure integration sets the same parameter at every
  // integration point.
  if (parameter == distribution_.getParameter())
    return;
  Distribution candidate(cloneDistribution(distribution_));
  candidate.setParameter(parameter);
  distribution_ = candidate;
  invalidateCache();
  refreshEvaluationDelegates();
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
  invalidateCache();
  refreshEvaluationDelegates();
}

} /* namespace OTROBOPT */
