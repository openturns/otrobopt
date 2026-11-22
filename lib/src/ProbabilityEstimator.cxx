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
#include "otrobopt/ProbabilityEstimator.hxx"
#include <openturns/Exception.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(ProbabilityEstimator)

/* Default constructor */
ProbabilityEstimator::ProbabilityEstimator()
  : TypedInterfaceObject<ProbabilityEstimatorImplementation>()
{
  // Nothing to do
}

/* Constructor from an implementation */
ProbabilityEstimator::ProbabilityEstimator(const ProbabilityEstimatorImplementation & implementation)
  : TypedInterfaceObject<ProbabilityEstimatorImplementation>(implementation.clone())
{
  // Nothing to do
}

/* Constructor from a pointer to an implementation */
ProbabilityEstimator::ProbabilityEstimator(const Implementation & p_implementation)
  : TypedInterfaceObject<ProbabilityEstimatorImplementation>(p_implementation)
{
  // Nothing to do
}

/* Probability of the event at the design point */
Scalar ProbabilityEstimator::getSafeProbability(const Function & predicate,
                                                const Function & limitState,
                                                const Distribution & distribution,
                                                const Point & designPoint) const
{
  if (getImplementation().isNull())
    throw InvalidArgumentException(HERE) << "The probability estimator has no implementation";
  return getImplementation()->getSafeProbability(predicate, limitState, distribution, designPoint);
}

/* Error estimate of the probability, NaN if undefined */
Scalar ProbabilityEstimator::getEstimationError(const Function & predicate,
                                                const Function & limitState,
                                                const Distribution & distribution,
                                                const Point & designPoint) const
{
  if (getImplementation().isNull())
    throw InvalidArgumentException(HERE) << "The probability estimator has no implementation";
  return getImplementation()->getEstimationError(predicate, limitState, distribution, designPoint);
}

/* Whether the estimator provides a geometry */
Bool ProbabilityEstimator::hasGeometry() const
{
  if (getImplementation().isNull())
    throw InvalidArgumentException(HERE) << "The probability estimator has no implementation";
  return getImplementation()->hasGeometry();
}

/* Signed reliability index of each limit-state component */
Point ProbabilityEstimator::getComponentReliabilityIndexes(const Function & predicate,
                                                           const Function & limitState,
                                                           const Distribution & distribution,
                                                           const Point & designPoint) const
{
  if (getImplementation().isNull())
    throw InvalidArgumentException(HERE) << "The probability estimator has no implementation";
  return getImplementation()->getComponentReliabilityIndexes(predicate, limitState, distribution, designPoint);
}

/* Standard-space design points, grouped by limit-state component */
Sample ProbabilityEstimator::getStandardDesignPoints(const Function & predicate,
                                                     const Function & limitState,
                                                     const Distribution & distribution,
                                                     const Point & designPoint) const
{
  if (getImplementation().isNull())
    throw InvalidArgumentException(HERE) << "The probability estimator has no implementation";
  return getImplementation()->getStandardDesignPoints(predicate, limitState, distribution, designPoint);
}

/* Number of standard-space design points of each limit-state component */
Indices ProbabilityEstimator::getDesignPointsNumber(const Function & predicate,
                                                    const Function & limitState,
                                                    const Distribution & distribution,
                                                    const Point & designPoint) const
{
  if (getImplementation().isNull())
    throw InvalidArgumentException(HERE) << "The probability estimator has no implementation";
  return getImplementation()->getDesignPointsNumber(predicate, limitState, distribution, designPoint);
}

/* Bounds of the probability of the linearized event */
Interval ProbabilityEstimator::getProbabilityBounds(const Function & predicate,
                                                    const Function & limitState,
                                                    const Distribution & distribution,
                                                    const Point & designPoint) const
{
  if (getImplementation().isNull())
    throw InvalidArgumentException(HERE) << "The probability estimator has no implementation";
  return getImplementation()->getProbabilityBounds(predicate, limitState, distribution, designPoint);
}

/* String converter */
String ProbabilityEstimator::__repr__() const
{
  if (getImplementation().isNull())
    return "class=ProbabilityEstimator implementation=null";
  return getImplementation()->__repr__();
}

} /* namespace OTROBOPT */
