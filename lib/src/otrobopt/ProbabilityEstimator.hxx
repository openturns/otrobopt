//                                               -*- C++ -*-
/**
 *  @brief Handle to a system-event probability estimator
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
#ifndef OTROBOPT_PROBABILITYESTIMATOR_HXX
#define OTROBOPT_PROBABILITYESTIMATOR_HXX

#include <openturns/Indices.hxx>
#include <openturns/Interval.hxx>
#include <openturns/Sample.hxx>
#include <openturns/TypedInterfaceObject.hxx>

#include "otrobopt/ProbabilityEstimatorImplementation.hxx"

namespace OTROBOPT
{

/**
 * @class ProbabilityEstimator
 *
 * Handle to an estimator of the probability of a Boolean system event.
 *
 * The handle owns its implementation by a shared pointer, so that a measure
 * can be copied and persisted without slicing the concrete estimator.  The
 * four arguments of getSafeProbability and getEstimationError are the event
 * predicate, the parametric limit-state function, the distribution of its
 * parameter, and the design point.
 */
class OTROBOPT_API ProbabilityEstimator
  : public OT::TypedInterfaceObject<ProbabilityEstimatorImplementation>
{
  CLASSNAME

public:
  typedef OT::TypedInterfaceObject<ProbabilityEstimatorImplementation> TypedInterfaceObjectType;

  /** Default constructor */
  ProbabilityEstimator();

  /** Constructor from an implementation */
  ProbabilityEstimator(const ProbabilityEstimatorImplementation & implementation);

  /** Constructor from a pointer to an implementation */
  ProbabilityEstimator(const Implementation & p_implementation);

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
  OT::Bool hasGeometry() const;

  /** Signed reliability index of each limit-state component */
  OT::Point getComponentReliabilityIndexes(const OT::Function & predicate,
                                           const OT::Function & limitState,
                                           const OT::Distribution & distribution,
                                           const OT::Point & designPoint) const;

  /** Standard-space design points, grouped by limit-state component */
  OT::Sample getStandardDesignPoints(const OT::Function & predicate,
                                     const OT::Function & limitState,
                                     const OT::Distribution & distribution,
                                     const OT::Point & designPoint) const;

  /** Number of standard-space design points of each limit-state component */
  OT::Indices getDesignPointsNumber(const OT::Function & predicate,
                                    const OT::Function & limitState,
                                    const OT::Distribution & distribution,
                                    const OT::Point & designPoint) const;

  /** Bounds of the probability of the linearized event */
  OT::Interval getProbabilityBounds(const OT::Function & predicate,
                                    const OT::Function & limitState,
                                    const OT::Distribution & distribution,
                                    const OT::Point & designPoint) const;

  /** String converter */
  OT::String __repr__() const override;

}; /* class ProbabilityEstimator */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_PROBABILITYESTIMATOR_HXX */
