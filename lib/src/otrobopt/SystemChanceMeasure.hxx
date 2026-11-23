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
#ifndef OTROBOPT_SYSTEMCHANCEMEASURE_HXX
#define OTROBOPT_SYSTEMCHANCEMEASURE_HXX

#include "otrobopt/MeasureEvaluationImplementation.hxx"
#include "otrobopt/ProbabilityEstimator.hxx"
#include <openturns/ComparisonOperator.hxx>

namespace OTROBOPT
{

/**
 * @class SystemChanceMeasure
 *
 * Chance measure of a Boolean system event.
 *
 * The event is a Boolean predicate phi of the indicators of the m
 * limit-state components {G_j(x, theta) >= 0}, evaluated at the design point
 * x for the realization theta of the distribution.  The predicate is a
 * Function with m inputs, one output and no parameter; it must return 0 or 1.
 *
 * The probability of the event is delegated to a ProbabilityEstimator owned
 * by the measure.  The estimator receives the predicate, the parametric
 * limit-state function, the distribution and the design point at every call,
 * so the measure can be rebuilt over a new function and distribution without
 * replacing the estimator.
 *
 * As in JointChanceMeasure, alpha_ is the target probability of the safe
 * event and the comparison operator only orients the returned constraint:
 * for a Less operator the value is qNormal(alpha) - qNormal(P), otherwise it
 * is qNormal(P) - qNormal(alpha), where P is the event probability.
 */
class OTROBOPT_API SystemChanceMeasure
  : public MeasureEvaluationImplementation
{
  CLASSNAME

public:
  /** Default constructor */
  SystemChanceMeasure();

  /** Parameter constructor */
  SystemChanceMeasure(const OT::Function & limitState,
                      const OT::Function & eventPredicate,
                      const OT::Distribution & distribution,
                      const OT::ComparisonOperator & op,
                      const OT::Scalar alpha);

  /** Virtual constructor method */
  SystemChanceMeasure * clone() const override;

  /** Rebuild the measure over a new function and distribution */
  using MeasureEvaluationImplementation::rebuild;
  MeasureEvaluation rebuild(const OT::Function & function,
                            const OT::Distribution & distribution,
                            const OT::Interval & optimizationDomain) const override;

  /** Evaluation */
  OT::Point operator()(const OT::Point & designPoint) const override;

  /** Dimension accessor */
  OT::UnsignedInteger getOutputDimension() const override;

  /** Event predicate accessor */
  void setEventPredicate(const OT::Function & predicate);
  OT::Function getEventPredicate() const;

  /** Estimator accessor */
  void setEstimator(const ProbabilityEstimator & estimator);
  ProbabilityEstimator getEstimator() const;

  /** Probability of the stored event at the given design point */
  OT::Scalar getSafeProbability(const OT::Point & designPoint) const;

  /** Signed reliability index of the stored event: qNormal(P) */
  OT::Scalar getReliabilityIndex(const OT::Point & designPoint) const;

  /** Error estimate of the stored event probability, NaN if undefined */
  OT::Scalar getEstimationError(const OT::Point & designPoint) const;

  /** Whether the estimator provides a geometry */
  OT::Bool hasGeometry() const;

  /** Signed reliability index of each limit-state component */
  OT::Point getComponentReliabilityIndexes(const OT::Point & designPoint) const;

  /** Standard-space design points, grouped by limit-state component */
  OT::Sample getStandardDesignPoints(const OT::Point & designPoint) const;

  /** Number of standard-space design points of each limit-state component */
  OT::Indices getDesignPointsNumber(const OT::Point & designPoint) const;

  /** Bounds of the probability of the linearized event */
  OT::Interval getProbabilityBounds(const OT::Point & designPoint) const;

  /** Comparison operator accessor */
  void setOperator(const OT::ComparisonOperator & op);
  OT::ComparisonOperator getOperator() const;

  /** Target probability accessor */
  void setAlpha(const OT::Scalar alpha);
  OT::Scalar getAlpha() const;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

private:
  OT::Function eventPredicate_;
  ProbabilityEstimator estimator_;
  OT::ComparisonOperator operator_;
  OT::Scalar alpha_ = 0.0;

}; /* class SystemChanceMeasure */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_SYSTEMCHANCEMEASURE_HXX */
