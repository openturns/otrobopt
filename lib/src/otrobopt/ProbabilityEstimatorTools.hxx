//                                               -*- C++ -*-
/**
 *  @brief Internal evaluation helpers for the probability estimators
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
#ifndef OTROBOPT_PROBABILITYESTIMATORTOOLS_HXX
#define OTROBOPT_PROBABILITYESTIMATORTOOLS_HXX

#include <openturns/Distribution.hxx>
#include <openturns/Function.hxx>
#include <openturns/Indices.hxx>
#include <openturns/Point.hxx>

#include "otrobopt/OTRobOptprivate.hxx"

namespace OTROBOPT
{
namespace ProbabilityEstimatorTools
{

/* Assemble the full input point of a parametric evaluation from the input
   point and the parameter theta. */
OTROBOPT_LOCAL OT::Point buildFullPoint(const OT::Point & inputPoint,
                                        const OT::Point & theta,
                                        const OT::Indices & inputPositions,
                                        const OT::Indices & parametersPositions,
                                        const OT::UnsignedInteger fullDimension);

/* Map the limit-state components to their Boolean indicators: 1 when the
   component is nonnegative (safe), 0 otherwise. */
OTROBOPT_LOCAL OT::Point buildIndicators(const OT::Point & limitStateValue);

/* Evaluate the limit-state function at the design point for the distribution
   parameter theta.  A parametric evaluation is evaluated on the assembled
   point, which avoids cloning the function for every integration node. */
OTROBOPT_LOCAL OT::Point evaluateLimitState(const OT::Function & limitState,
                                            const OT::Point & designPoint,
                                            const OT::Point & theta);

/* Evaluate a Boolean predicate and reject any value other than 0 or 1. */
OTROBOPT_LOCAL OT::Scalar evaluatePredicate(const OT::Function & predicate,
                                            const OT::Point & indicators);

/* Validate the dimensions and the Boolean predicate of an estimation.  The
   predicate is probed on zero indicators: a necessary condition only, since
   every value used by an estimator is checked by evaluatePredicate(). */
OTROBOPT_LOCAL void checkArguments(const OT::Function & predicate,
                                   const OT::Function & limitState,
                                   const OT::Distribution & distribution,
                                   const OT::Point & designPoint);

/* Freeze the design point of a parametric limit state: return the function
   theta -> G(designPoint, theta) and its parameter gradient.  This is the
   random-side function of an inner reliability analysis at a fixed design. */
OTROBOPT_LOCAL OT::Function buildFrozenLimitState(const OT::Function & limitState,
                                                  const OT::Point & designPoint);

} /* namespace ProbabilityEstimatorTools */
} /* namespace OTROBOPT */

#endif /* OTROBOPT_PROBABILITYESTIMATORTOOLS_HXX */
