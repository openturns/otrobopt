//                                               -*- C++ -*-
/**
 *  @brief Quadrature estimator of a system-event probability
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
#include "otrobopt/QuadratureEstimator.hxx"
#include "otrobopt/ProbabilityEstimatorTools.hxx"
#include <openturns/Description.hxx>
#include <openturns/Exception.hxx>
#include <openturns/FunctionImplementation.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/GaussKronrodRule.hxx>
#include <openturns/IteratedQuadrature.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/ResourceMap.hxx>
#include <openturns/SpecFunc.hxx>

#include <limits>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(QuadratureEstimator)

static Factory<QuadratureEstimator> Factory_QuadratureEstimator;

namespace
{

/* Lebesgue integrand of the continuous branch: p(theta) times the Boolean
   predicate evaluated on the indicators of the limit-state components. */
class QuadratureEstimatorIntegrand
  : public FunctionImplementation
{
public:
  QuadratureEstimatorIntegrand(const Point & designPoint,
                               const Function & limitState,
                               const Function & predicate,
                               const Distribution & distribution,
                               const Scalar pdfThreshold)
    : FunctionImplementation()
    , designPoint_(designPoint)
    , limitState_(limitState)
    , predicate_(predicate)
    , distribution_(distribution)
    , pdfThreshold_(pdfThreshold)
  {
    // Nothing to do
  }

  QuadratureEstimatorIntegrand * clone() const override
  {
    return new QuadratureEstimatorIntegrand(*this);
  }

  Point operator()(const Point & theta) const override
  {
    const Scalar pdf = distribution_.computePDF(theta);
    if (pdf <= pdfThreshold_)
      return Point(1, 0.0);
    const Point value(ProbabilityEstimatorTools::evaluateLimitState(limitState_, designPoint_, theta));
    const Point indicators(ProbabilityEstimatorTools::buildIndicators(value));
    const Scalar booleanValue = ProbabilityEstimatorTools::evaluatePredicate(predicate_, indicators);
    return Point(1, booleanValue * pdf);
  }

  Sample operator()(const Sample & theta) const override
  {
    const UnsignedInteger size = theta.getSize();
    Sample result(size, 1);
    for (UnsignedInteger i = 0; i < size; ++i)
      result(i, 0) = operator()(theta[i])[0];
    return result;
  }

  UnsignedInteger getInputDimension() const override
  {
    return limitState_.getParameterDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return 1;
  }

  Description getInputDescription() const override
  {
    return limitState_.getParameterDescription();
  }

  Description getOutputDescription() const override
  {
    return Description(1, "P");
  }

private:
  Point designPoint_;
  Function limitState_;
  Function predicate_;
  Distribution distribution_;
  Scalar pdfThreshold_;
};

}

/* Default constructor */
QuadratureEstimator::QuadratureEstimator()
  : ProbabilityEstimatorImplementation()
  , pdfThreshold_(SpecFunc::Precision)
{
  GaussKronrod gkr;
  gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("QuadratureEstimator-GaussKronrodRule")));
  integrationAlgorithm_ = IteratedQuadrature(gkr);
}

/* Constructor from an integration algorithm */
QuadratureEstimator::QuadratureEstimator(const IntegrationAlgorithm & algorithm)
  : ProbabilityEstimatorImplementation()
  , integrationAlgorithm_(algorithm)
  , pdfThreshold_(SpecFunc::Precision)
{
  // Nothing to do
}

/* Virtual constructor method */
QuadratureEstimator * QuadratureEstimator::clone() const
{
  return new QuadratureEstimator(*this);
}

/* Integration algorithm accessor */
void QuadratureEstimator::setIntegrationAlgorithm(const IntegrationAlgorithm & algorithm)
{
  integrationAlgorithm_ = algorithm;
}

IntegrationAlgorithm QuadratureEstimator::getIntegrationAlgorithm() const
{
  return integrationAlgorithm_;
}

/* Density threshold accessor */
void QuadratureEstimator::setPDFThreshold(const Scalar threshold)
{
  if (!(threshold >= 0.0))
    throw InvalidArgumentException(HERE) << "The PDF threshold must be nonnegative, got " << threshold;
  pdfThreshold_ = threshold;
}

Scalar QuadratureEstimator::getPDFThreshold() const
{
  return pdfThreshold_;
}

/* Probability implementation */
Scalar QuadratureEstimator::computeSafeProbability(const Function & predicate,
                                                   const Function & limitState,
                                                   const Distribution & distribution,
                                                   const Point & designPoint) const
{
  if (distribution.isContinuous())
  {
    Pointer<FunctionImplementation> p_integrand(new QuadratureEstimatorIntegrand(designPoint, limitState, predicate, distribution, pdfThreshold_));
    const Function integrand(p_integrand);
    const Point value(integrationAlgorithm_.integrate(integrand, distribution.getRange()));
    return value[0];
  }
  if (!distribution.isDiscrete())
    throw NotYetImplementedException(HERE) << "The quadrature estimator handles continuous or discrete distributions, got "
                                           << distribution.getClassName();

  const Point probabilities(distribution.getProbabilities());
  const Sample support(distribution.getSupport());
  if (support.getSize() != probabilities.getSize())
    throw InternalException(HERE) << "The distribution support size (" << support.getSize()
                                  << ") does not match the probability size (" << probabilities.getSize() << ")";

  // Exact sum over the support: dropping a low-probability atom would change
  // the probability instead of truncating a density tail.
  Scalar probability = 0.0;
  for (UnsignedInteger i = 0; i < support.getSize(); ++i)
  {
    const Point value(ProbabilityEstimatorTools::evaluateLimitState(limitState, designPoint, support[i]));
    const Point indicators(ProbabilityEstimatorTools::buildIndicators(value));
    if (ProbabilityEstimatorTools::evaluatePredicate(predicate, indicators) != 0.0)
      probability += probabilities[i];
  }
  return probability;
}

/* Error implementation: no quadrature gap is available */
Scalar QuadratureEstimator::computeEstimationError(const Function & /*predicate*/,
                                                   const Function & /*limitState*/,
                                                   const Distribution & /*distribution*/,
                                                   const Point & /*designPoint*/) const
{
  return std::numeric_limits<Scalar>::quiet_NaN();
}

/* String converter */
String QuadratureEstimator::__repr__() const
{
  OSS oss;
  oss << "class=" << QuadratureEstimator::GetClassName()
      << " algorithm=" << integrationAlgorithm_
      << " pdfThreshold=" << pdfThreshold_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void QuadratureEstimator::save(Advocate & adv) const
{
  ProbabilityEstimatorImplementation::save(adv);
  adv.saveAttribute("integrationAlgorithm_", integrationAlgorithm_);
  adv.saveAttribute("pdfThreshold_", pdfThreshold_);
}

/* Method load() reloads the object from the StorageManager */
void QuadratureEstimator::load(Advocate & adv)
{
  ProbabilityEstimatorImplementation::load(adv);
  adv.loadAttribute("integrationAlgorithm_", integrationAlgorithm_);
  adv.loadAttribute("pdfThreshold_", pdfThreshold_);
}

} /* namespace OTROBOPT */
