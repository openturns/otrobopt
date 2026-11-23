//                                               -*- C++ -*-
/**
 *  @brief Base class of the system-event probability estimators
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
#ifndef OTROBOPT_PROBABILITYESTIMATORIMPLEMENTATION_HXX
#define OTROBOPT_PROBABILITYESTIMATORIMPLEMENTATION_HXX

#include <openturns/Distribution.hxx>
#include <openturns/Function.hxx>
#include <openturns/Indices.hxx>
#include <openturns/Interval.hxx>
#include <openturns/PersistentObject.hxx>
#include <openturns/Point.hxx>
#include <openturns/Sample.hxx>

#include "otrobopt/OTRobOptprivate.hxx"

namespace OTROBOPT
{

/**
 * @class ProbabilityEstimatorImplementation
 *
 * Implementation base class of the estimators of the probability of a Boolean
 * system event.
 *
 * The event is described by a Boolean predicate phi from {0,1}^m to {0,1},
 * evaluated on the indicators of the m limit-state components
 * {G_j(x, theta) >= 0}.  The predicate is a Function with m inputs, one
 * output and no parameter.  The limit state is a parametric Function from
 * R^{d_x} to R^m whose parameter is the realization theta of the distribution
 * D.  The estimator is stateless: every call receives the predicate, the
 * limit-state function, the distribution and the design point, so that it can
 * be shared by several measures and by rebuilt measures.
 *
 * The probability is returned before the probit transform of the chance
 * measure.  getEstimationError returns NaN when the estimator has no error
 * estimate for the requested call.
 */
class OTROBOPT_API ProbabilityEstimatorImplementation
  : public OT::PersistentObject
{
  CLASSNAME

public:
  /** Default constructor */
  ProbabilityEstimatorImplementation();

  /** Virtual constructor, with a covariant return type for the handle */
  ProbabilityEstimatorImplementation * clone() const override = 0;

  /** Probability of the event at the design point */
  OT::Scalar getSafeProbability(const OT::Function & predicate,
                                const OT::Function & limitState,
                                const OT::Distribution & distribution,
                                const OT::Point & designPoint) const;

  /** Error estimate of the probability, NaN if undefined */
  OT::Scalar getEstimationError(const OT::Function & predicate,
                                const OT::Function & limitState,
                                const OT::Distribution & distribution,
                                const OT::Point & designPoint) const;

  /** Whether the estimator provides a geometry (design points, bounds) */
  virtual OT::Bool hasGeometry() const;

  /** Signed reliability index of each limit-state component.  The default
      implementation throws: only the estimators based on a reliability
      analysis provide a per-component geometry. */
  virtual OT::Point getComponentReliabilityIndexes(const OT::Function & predicate,
                                                   const OT::Function & limitState,
                                                   const OT::Distribution & distribution,
                                                   const OT::Point & designPoint) const;

  /** Standard-space design points, grouped by limit-state component.  The
      default implementation throws. */
  virtual OT::Sample getStandardDesignPoints(const OT::Function & predicate,
                                             const OT::Function & limitState,
                                             const OT::Distribution & distribution,
                                             const OT::Point & designPoint) const;

  /** Number of standard-space design points of each limit-state component.
      The default implementation throws. */
  virtual OT::Indices getDesignPointsNumber(const OT::Function & predicate,
                                            const OT::Function & limitState,
                                            const OT::Distribution & distribution,
                                            const OT::Point & designPoint) const;

  /** Bounds of the probability of the linearized event.  The default
      implementation throws. */
  virtual OT::Interval getProbabilityBounds(const OT::Function & predicate,
                                            const OT::Function & limitState,
                                            const OT::Distribution & distribution,
                                            const OT::Point & designPoint) const;

  /** String converter */
  OT::String __repr__() const override;

protected:
  /** Probability implementation */
  virtual OT::Scalar computeSafeProbability(const OT::Function & predicate,
                                            const OT::Function & limitState,
                                            const OT::Distribution & distribution,
                                            const OT::Point & designPoint) const = 0;

  /** Error implementation */
  virtual OT::Scalar computeEstimationError(const OT::Function & predicate,
                                            const OT::Function & limitState,
                                            const OT::Distribution & distribution,
                                            const OT::Point & designPoint) const = 0;

}; /* class ProbabilityEstimatorImplementation */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_PROBABILITYESTIMATORIMPLEMENTATION_HXX */
