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

/** Quiet NaN sentinel */
OTROBOPT_API extern const OT::Scalar NaN;

/** Result of one inner reliability analysis */
struct OTROBOPT_API InnerReport
{
  OT::Scalar constraint = 0.0;
  OT::Scalar beta = 0.0;
  OT::Scalar performance = 0.0;
  OT::Point standardDesignPoint;
};

/** One solved reliability component, shared by the specialized algorithms */
struct OTROBOPT_API ReliabilityReport
{
  InnerReport inner;
  OT::UnsignedInteger component = 0;
  OT::Point physicalDesignPoint;
};

/** Whether a point has only finite coordinates */
OTROBOPT_API OT::Bool IsFinitePoint(const OT::Point & point);

/** Safe event sign: +1 for Greater* and -1 for Less* operators */
OTROBOPT_API OT::Scalar GetSafeSign(const OT::ComparisonOperator & op);

/** Target probability of the safe event associated with the operator */
OTROBOPT_API OT::Scalar GetSafeProbability(const OT::ComparisonOperator & op,
                                           const OT::Scalar alpha);

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
