//                                               -*- C++ -*-
/**
 *  @brief Shared helpers for the specialized RBDO algorithms
 *
 *  Copyright 2005-2026 Airbus-EDF-IMACS-ONERA-Phimeca
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef OTROBOPT_RBDOALGORITHMTOOLS_HXX
#define OTROBOPT_RBDOALGORITHMTOOLS_HXX

#include <openturns/Collection.hxx>
#include <openturns/ComparisonOperator.hxx>
#include <openturns/Distribution.hxx>
#include <openturns/Function.hxx>
#include <openturns/OptimizationAlgorithm.hxx>
#include <openturns/OTtypes.hxx>
#include <openturns/Point.hxx>

#include "otrobopt/OTRobOptprivate.hxx"
#include "otrobopt/RBDOProblem.hxx"

namespace OTROBOPT
{

/**
 * Internal helpers shared by the specialized RBDO algorithms.  This header is
 * not installed: it is an implementation detail of the module.
 */
namespace RBDOAlgorithmTools
{

/** Result of one inner reliability analysis */
struct OTROBOPT_API InnerReport
{
  OT::Scalar constraint = 0.0;
  OT::Scalar beta = 0.0;
  OT::Scalar performance = 0.0;
  OT::Point standardDesignPoint;
};

/** Whether a point has only finite coordinates */
OTROBOPT_API OT::Bool IsFinitePoint(const OT::Point & point);

/** Safe event sign: +1 for Greater* and -1 for Less* operators */
OTROBOPT_API OT::Scalar GetSafeSign(const OT::ComparisonOperator & op);

/** Target probability of the safe event associated with the operator */
OTROBOPT_API OT::Scalar GetSafeProbability(const OT::ComparisonOperator & op,
                                           const OT::Scalar alpha);

/** One-dimensional marginal of the standard distribution of a distribution.
    For an elliptical distribution this is the same-kind standard marginal
    (e.g. Student(nu) for a Student distribution), i.e. the marginal of the
    generalized Nataf standard space; otherwise the standard distribution is
    Normal and this is the standard normal law. */
OTROBOPT_API OT::Distribution GetStandardMarginal(const OT::Distribution & distribution);

/** Reliability index in the standard space associated with a safe probability
    in (0.5, 1).  It is the quantile of the standard marginal, so that the
    half-space at that distance has the requested safe probability. */
OTROBOPT_API OT::Scalar ProbabilityToIndex(const OT::Distribution & standardMarginal,
                                           const OT::Scalar safeProbability);

/** FORM approximation of the safe probability from a signed reliability index
    in the standard space. */
OTROBOPT_API OT::Scalar IndexToProbability(const OT::Distribution & standardMarginal,
                                           const OT::Scalar reliabilityIndex);

/** Standard-space vector limit state for a given design point */
OTROBOPT_API OT::Function BuildStandardLimitState(const RBDOProblem & problem,
                                                  const OT::Point & designPoint,
                                                  const OT::Function & inverseTransformation);

/** Scalarization of a vector limit state; an output index equal to the output
    dimension selects the exact joint event. */
OTROBOPT_API OT::Function BuildScalarizedFunction(const OT::Function & vectorFunction,
                                                  const OT::Scalar safeSign,
                                                  const OT::UnsignedInteger outputIndex);

/** Starting points of an inner reliability analysis */
OTROBOPT_API OT::Collection<OT::Point> BuildStartingPoints(const OT::Function & function,
                                                           const OT::Scalar betaTarget,
                                                           const OT::Point & warmStart,
                                                           const OT::Bool forPMA);

/** Evaluate a scalarized limit state at the origin of the standard space */
OTROBOPT_API OT::Scalar OriginValue(const OT::Function & function);

/** Check the residual of an inner reliability analysis */
OTROBOPT_API void CheckResidual(const OT::OptimizationAlgorithm & solver,
                                const OT::Scalar residual,
                                const OT::String & analysis);

/** RIA inner analysis */
OTROBOPT_API InnerReport SolveRIA(const OT::Function & function,
                                  const OT::Scalar betaTarget,
                                  const OT::OptimizationAlgorithm & innerSolver,
                                  const OT::Point & warmStart);

/** PMA inner analysis */
OTROBOPT_API InnerReport SolvePMA(const OT::Function & function,
                                  const OT::Scalar betaTarget,
                                  const OT::OptimizationAlgorithm & innerSolver,
                                  const OT::Point & warmStart);

/** Build the default inner solver from the ResourceMap keys prefixed by
    keyPrefix; for example keyPrefix="DoubleLoopRBDOAlgorithm". */
OTROBOPT_API OT::OptimizationAlgorithm BuildDefaultInnerSolver(const OT::String & keyPrefix);

} /* namespace RBDOAlgorithmTools */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_RBDOALGORITHMTOOLS_HXX */
