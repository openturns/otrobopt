//                                               -*- C++ -*-
/**
 *  @brief Reliability-based design optimization problem
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
#ifndef OTROBOPT_RBDOPROBLEM_HXX
#define OTROBOPT_RBDOPROBLEM_HXX

#include <openturns/OptimizationProblemImplementation.hxx>

#include "otrobopt/JointChanceMeasure.hxx"
#include "otrobopt/RobustOptimizationProblemCanonicalizer.hxx"

namespace OTROBOPT
{

/**
 * @class RBDOProblem
 *
 * Reliability-based design optimization problem.
 *
 * The problem has a scalar classical objective and one joint chance
 * constraint.  Deterministic bounds, inequality constraints and equality
 * constraints are handled by the OptimizationProblem protocol.  The
 * distribution parameter is frozen by default; passing canonicalization
 * options with optimizeParameter set to true exposes it as a decision
 * variable.
 */
class OTROBOPT_API RBDOProblem
  : public OT::OptimizationProblemImplementation
{
  CLASSNAME

public:
  /** Default constructor */
  RBDOProblem();

  /** Parameter constructor */
  RBDOProblem(const OT::Function & objective,
              const JointChanceMeasure & jointChanceMeasure);

  /** Virtual constructor method */
  RBDOProblem * clone() const override;

  /** Objective function accessor */
  void setObjective(const OT::Function & objective);

  /** Joint chance measure accessors */
  void setJointChanceMeasure(const JointChanceMeasure & jointChanceMeasure);
  JointChanceMeasure getJointChanceMeasure() const;
  OT::Bool hasJointChanceMeasure() const;

  /** Limit-state function accessor */
  OT::Function getLimitStateFunction() const;

  /** Distribution accessor */
  OT::Distribution getDistribution() const;

  /** Target probability accessor */
  OT::Scalar getTargetProbability() const;

  /** Comparison operator accessor */
  OT::ComparisonOperator getComparisonOperator() const;

  /** Check the problem contract */
  void check() const;

  /** Default canonicalization options: the distribution parameter is frozen */
  static CanonicalizationOptions GetDefaultCanonicalizationOptions();

  /** Canonicalizer accessors */
  RobustOptimizationProblemCanonicalizer getCanonicalizer() const;
  RobustOptimizationProblemCanonicalizer getCanonicalizer(
    const CanonicalizationOptions & options) const;

  /** Solver-ready canonical problem accessors */
  RobustOptimizationProblem getCanonicalProblem() const;
  RobustOptimizationProblem getCanonicalProblem(
    const CanonicalizationOptions & options) const;

  /** Canonical joint chance measure accessors */
  JointChanceMeasure getCanonicalJointChanceMeasure() const;
  JointChanceMeasure getCanonicalJointChanceMeasure(
    const CanonicalizationOptions & options) const;

  /** Canonical deterministic inequality constraint accessors */
  OT::Function getCanonicalDeterministicConstraint() const;
  OT::Function getCanonicalDeterministicConstraint(
    const CanonicalizationOptions & options) const;

  /** Canonical starting point accessors */
  OT::Point getCanonicalStartingPoint() const;
  OT::Point getCanonicalStartingPoint(
    const CanonicalizationOptions & options) const;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

private:
  RobustOptimizationProblem buildRobustProblem() const;
  OT::Function liftDeterministicConstraint(
    const RobustOptimizationProblemCanonicalizer & canonicalizer) const;

  JointChanceMeasure jointChanceMeasure_;
}; /* class RBDOProblem */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_RBDOPROBLEM_HXX */
