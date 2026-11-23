//                                               -*- C++ -*-
/**
 *  @brief System chance measure
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
#include "otrobopt/SystemChanceMeasure.hxx"
#include "otrobopt/MeasureEvaluation.hxx"
#include "otrobopt/ProbabilityEstimatorTools.hxx"
#include "otrobopt/QuadratureEstimator.hxx"
#include <openturns/DistFunc.hxx>
#include <openturns/PersistentObjectFactory.hxx>

#include <algorithm>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(SystemChanceMeasure)

static Factory<SystemChanceMeasure> Factory_SystemChanceMeasure;

/* Default constructor */
SystemChanceMeasure::SystemChanceMeasure()
  : MeasureEvaluationImplementation()
  , estimator_(QuadratureEstimator())
  , alpha_(0.0)
{
  // Nothing to do
}

/* Parameter constructor */
SystemChanceMeasure::SystemChanceMeasure(const Function & limitState,
                                         const Function & eventPredicate,
                                         const Distribution & distribution,
                                         const ComparisonOperator & op,
                                         const Scalar alpha)
  : MeasureEvaluationImplementation(limitState, distribution)
  , eventPredicate_(eventPredicate)
  , estimator_(QuadratureEstimator())
  , operator_(op)
{
  setOutputDescription(Description(1, "P"));
  setAlpha(alpha);
  // Fail at construction on an inconsistent event rather than at the first
  // evaluation: the checks are independent of the design point.
  ProbabilityEstimatorTools::checkArguments(eventPredicate_, getFunction(), getDistribution(),
                                            Point(getFunction().getInputDimension()));
}

/* Virtual constructor method */
SystemChanceMeasure * SystemChanceMeasure::clone() const
{
  return new SystemChanceMeasure(*this);
}

/* Rebuild the measure over a new function and distribution */
MeasureEvaluation SystemChanceMeasure::rebuild(const Function & function,
                                               const Distribution & distribution,
                                               const Interval & /*optimizationDomain*/) const
{
  SystemChanceMeasure result(function, eventPredicate_, distribution, operator_, alpha_);
  result.setEstimator(estimator_);
  result.setPDFThreshold(getPDFThreshold());
  return result;
}

/* Evaluation */
Point SystemChanceMeasure::operator()(const Point & designPoint) const
{
  const Scalar beta = getReliabilityIndex(designPoint);
  Point outP(1);
  outP[0] = operator_.operator()(1.0, 2.0) ? DistFunc::qNormal(alpha_) - beta : beta - DistFunc::qNormal(alpha_);
  return outP;
}

/* Dimension accessor */
UnsignedInteger SystemChanceMeasure::getOutputDimension() const
{
  return 1;
}

/* Event predicate accessor */
void SystemChanceMeasure::setEventPredicate(const Function & predicate)
{
  eventPredicate_ = predicate;
}

Function SystemChanceMeasure::getEventPredicate() const
{
  return eventPredicate_;
}

/* Estimator accessor */
void SystemChanceMeasure::setEstimator(const ProbabilityEstimator & estimator)
{
  estimator_ = estimator;
}

ProbabilityEstimator SystemChanceMeasure::getEstimator() const
{
  return estimator_;
}

/* Probability of the stored event at the given design point */
Scalar SystemChanceMeasure::getSafeProbability(const Point & designPoint) const
{
  return estimator_.getSafeProbability(eventPredicate_, getFunction(), getDistribution(), designPoint);
}

/* Signed reliability index of the stored event */
Scalar SystemChanceMeasure::getReliabilityIndex(const Point & designPoint) const
{
  const Scalar probability = getSafeProbability(designPoint);
  // Same clipping as JointChanceMeasure: the probit must stay finite when the
  // estimator returns exactly 0 or 1.
  const Scalar threshold = getPDFThreshold();
  const Scalar clippedProbability = std::min(std::max(probability, threshold), 1.0 - threshold);
  return DistFunc::qNormal(clippedProbability);
}

/* Error estimate of the stored event probability */
Scalar SystemChanceMeasure::getEstimationError(const Point & designPoint) const
{
  return estimator_.getEstimationError(eventPredicate_, getFunction(), getDistribution(), designPoint);
}

/* Whether the estimator provides a geometry */
Bool SystemChanceMeasure::hasGeometry() const
{
  return estimator_.hasGeometry();
}

/* Signed reliability index of each limit-state component */
Point SystemChanceMeasure::getComponentReliabilityIndexes(const Point & designPoint) const
{
  return estimator_.getComponentReliabilityIndexes(eventPredicate_, getFunction(), getDistribution(), designPoint);
}

/* Standard-space design points, grouped by limit-state component */
Sample SystemChanceMeasure::getStandardDesignPoints(const Point & designPoint) const
{
  return estimator_.getStandardDesignPoints(eventPredicate_, getFunction(), getDistribution(), designPoint);
}

/* Number of standard-space design points of each limit-state component */
Indices SystemChanceMeasure::getDesignPointsNumber(const Point & designPoint) const
{
  return estimator_.getDesignPointsNumber(eventPredicate_, getFunction(), getDistribution(), designPoint);
}

/* Bounds of the probability of the linearized event */
Interval SystemChanceMeasure::getProbabilityBounds(const Point & designPoint) const
{
  return estimator_.getProbabilityBounds(eventPredicate_, getFunction(), getDistribution(), designPoint);
}

/* Comparison operator accessor */
void SystemChanceMeasure::setOperator(const ComparisonOperator & op)
{
  operator_ = op;
}

ComparisonOperator SystemChanceMeasure::getOperator() const
{
  return operator_;
}

/* Target probability accessor */
void SystemChanceMeasure::setAlpha(const Scalar alpha)
{
  if (!(alpha > 0.0) || !(alpha < 1.0))
    throw InvalidArgumentException(HERE) << "The target probability must be in (0, 1), got " << alpha;
  alpha_ = alpha;
}

Scalar SystemChanceMeasure::getAlpha() const
{
  return alpha_;
}

/* String converter */
String SystemChanceMeasure::__repr__() const
{
  OSS oss;
  oss << "class=" << SystemChanceMeasure::GetClassName()
      << " alpha=" << alpha_
      << " operator=" << operator_
      << " estimator=" << estimator_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void SystemChanceMeasure::save(Advocate & adv) const
{
  MeasureEvaluationImplementation::save(adv);
  adv.saveAttribute("eventPredicate_", eventPredicate_);
  adv.saveAttribute("estimator_", estimator_);
  adv.saveAttribute("operator_", operator_);
  adv.saveAttribute("alpha_", alpha_);
}

/* Method load() reloads the object from the StorageManager */
void SystemChanceMeasure::load(Advocate & adv)
{
  MeasureEvaluationImplementation::load(adv);
  adv.loadAttribute("eventPredicate_", eventPredicate_);
  adv.loadAttribute("estimator_", estimator_);
  adv.loadAttribute("operator_", operator_);
  adv.loadAttribute("alpha_", alpha_);
}

} /* namespace OTROBOPT */
