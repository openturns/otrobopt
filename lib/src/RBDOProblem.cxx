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
#include "otrobopt/RBDOProblem.hxx"

#include <openturns/AggregatedFunction.hxx>
#include <openturns/ComposedFunction.hxx>
#include <openturns/Exception.hxx>
#include <openturns/LinearFunction.hxx>
#include <openturns/Matrix.hxx>
#include <openturns/PersistentObjectFactory.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(RBDOProblem)

static Factory<RBDOProblem> Factory_RBDOProblem;

/* Default constructor */
RBDOProblem::RBDOProblem()
  : OptimizationProblemImplementation()
  , jointChanceMeasure_()
{
  // Nothing to do
}

/* Parameter constructor */
RBDOProblem::RBDOProblem(const Function & objective,
                         const JointChanceMeasure & jointChanceMeasure)
  : OptimizationProblemImplementation()
  , jointChanceMeasure_()
{
  setObjective(objective);
  setJointChanceMeasure(jointChanceMeasure);
  check();
}

/* Virtual constructor method */
RBDOProblem * RBDOProblem::clone() const
{
  return new RBDOProblem(*this);
}

/* Objective function accessor */
void RBDOProblem::setObjective(const Function & objective)
{
  if ((objective.getInputDimension() > 0) && (objective.getOutputDimension() != 1))
    throw InvalidDimensionException(HERE) << "The RBDO objective must be scalar, got output dimension "
                                          << objective.getOutputDimension();
  if ((objective.getInputDimension() > 0)
      && hasJointChanceMeasure()
      && (objective.getInputDimension() != getLimitStateFunction().getInputDimension()))
    throw InvalidDimensionException(HERE) << "The objective input dimension ("
                                          << objective.getInputDimension()
                                          << ") must match the limit-state function input dimension ("
                                          << getLimitStateFunction().getInputDimension() << ")";
  OptimizationProblemImplementation::setObjective(objective);
}

/* Joint chance measure accessors */
void RBDOProblem::setJointChanceMeasure(const JointChanceMeasure & jointChanceMeasure)
{
  const Function limitStateFunction(jointChanceMeasure.getFunction());
  if (limitStateFunction.getInputDimension() == 0)
    throw InvalidArgumentException(HERE) << "The joint chance measure must define a limit-state function";
  if (limitStateFunction.getOutputDimension() == 0)
    throw InvalidArgumentException(HERE) << "The limit-state function must have at least one output";
  if ((getObjective().getInputDimension() > 0)
      && (limitStateFunction.getInputDimension() != getObjective().getInputDimension()))
    throw InvalidDimensionException(HERE) << "The limit-state function input dimension ("
                                          << limitStateFunction.getInputDimension()
                                          << ") must match the objective input dimension ("
                                          << getObjective().getInputDimension() << ")";
  const Distribution distribution(jointChanceMeasure.getDistribution());
  if (distribution.getDimension() == 0)
    throw InvalidArgumentException(HERE) << "The joint chance measure must define a distribution";
  if (!distribution.isContinuous())
    throw InvalidArgumentException(HERE) << "Only continuous joint chance distributions are supported, got "
                                         << distribution.getImplementation()->getClassName();
  if (limitStateFunction.getParameterDimension() != distribution.getDimension())
    throw InvalidDimensionException(HERE) << "The limit-state function parameter dimension ("
                                          << limitStateFunction.getParameterDimension()
                                          << ") must match the distribution dimension ("
                                          << distribution.getDimension() << ")";
  jointChanceMeasure_ = jointChanceMeasure;
}

JointChanceMeasure RBDOProblem::getJointChanceMeasure() const
{
  if (!hasJointChanceMeasure())
    throw InvalidArgumentException(HERE) << "No joint chance measure defined";
  return jointChanceMeasure_;
}

Bool RBDOProblem::hasJointChanceMeasure() const
{
  return jointChanceMeasure_.getFunction().getInputDimension() > 0;
}

/* Limit-state function accessor */
Function RBDOProblem::getLimitStateFunction() const
{
  return getJointChanceMeasure().getFunction();
}

/* Distribution accessor */
Distribution RBDOProblem::getDistribution() const
{
  return getJointChanceMeasure().getDistribution();
}

/* Target probability accessor */
Scalar RBDOProblem::getTargetProbability() const
{
  return getJointChanceMeasure().getAlpha();
}

/* Comparison operator accessor */
ComparisonOperator RBDOProblem::getComparisonOperator() const
{
  return getJointChanceMeasure().getOperator();
}

/* Check the problem contract */
void RBDOProblem::check() const
{
  const Function objective(getObjective());
  if (objective.getInputDimension() == 0)
    throw InvalidArgumentException(HERE) << "The RBDO problem has no objective";
  if (objective.getOutputDimension() != 1)
    throw InvalidDimensionException(HERE) << "The RBDO objective must be scalar, got output dimension "
                                          << objective.getOutputDimension();
  if (!hasJointChanceMeasure())
    throw InvalidArgumentException(HERE) << "The RBDO problem has no joint chance measure";
  const Function limitStateFunction(getLimitStateFunction());
  if (limitStateFunction.getInputDimension() != objective.getInputDimension())
    throw InvalidDimensionException(HERE) << "The limit-state function input dimension ("
                                          << limitStateFunction.getInputDimension()
                                          << ") must match the objective input dimension ("
                                          << objective.getInputDimension() << ")";
  const Distribution distribution(getDistribution());
  if (distribution.getDimension() == 0)
    throw InvalidArgumentException(HERE) << "The joint chance measure must define a distribution";
  if (!distribution.isContinuous())
    throw InvalidArgumentException(HERE) << "Only continuous joint chance distributions are supported, got "
                                         << distribution.getImplementation()->getClassName();
  if (limitStateFunction.getParameterDimension() != distribution.getDimension())
    throw InvalidDimensionException(HERE) << "The limit-state function parameter dimension ("
                                          << limitStateFunction.getParameterDimension()
                                          << ") must match the distribution dimension ("
                                          << distribution.getDimension() << ")";
  if (hasInequalityConstraint()
      && (getInequalityConstraint().getInputDimension() != objective.getInputDimension()))
    throw InvalidDimensionException(HERE) << "The deterministic inequality constraint input dimension ("
                                          << getInequalityConstraint().getInputDimension()
                                          << ") must match the objective input dimension ("
                                          << objective.getInputDimension() << ")";
  if (hasEqualityConstraint()
      && (getEqualityConstraint().getInputDimension() != objective.getInputDimension()))
    throw InvalidDimensionException(HERE) << "The deterministic equality constraint input dimension ("
                                          << getEqualityConstraint().getInputDimension()
                                          << ") must match the objective input dimension ("
                                          << objective.getInputDimension() << ")";
  if ((getBounds().getDimension() != 0)
      && (getBounds().getDimension() != objective.getInputDimension()))
    throw InvalidDimensionException(HERE) << "The bounds dimension ("
                                          << getBounds().getDimension()
                                          << ") must match the objective input dimension ("
                                          << objective.getInputDimension() << ")";
}

/* Default canonicalization options */
CanonicalizationOptions RBDOProblem::GetDefaultCanonicalizationOptions()
{
  CanonicalizationOptions options;
  options.setOptimizeParameter(false);
  return options;
}

/* Build the generic robust problem used by the canonicalizer */
RobustOptimizationProblem RBDOProblem::buildRobustProblem() const
{
  RobustOptimizationProblem problem;
  problem.setObjective(getObjective());
  problem.setReliabilityMeasure(getJointChanceMeasure());
  if (getBounds().getDimension() != 0)
    problem.setBounds(getBounds());
  if (hasEqualityConstraint())
    problem.setEqualityConstraint(getEqualityConstraint());
  if (getVariablesType().getSize() != 0)
    problem.setVariablesType(getVariablesType());
  problem.setMinimization(isMinimization(0), 0);
  return problem;
}

/* Canonicalizer accessors */
RobustOptimizationProblemCanonicalizer RBDOProblem::getCanonicalizer() const
{
  return getCanonicalizer(GetDefaultCanonicalizationOptions());
}

RobustOptimizationProblemCanonicalizer RBDOProblem::getCanonicalizer(
  const CanonicalizationOptions & options) const
{
  check();
  return RobustOptimizationProblemCanonicalizer(buildRobustProblem(), options);
}

/* Lift a deterministic constraint of x to a constraint of (x, s) */
Function RBDOProblem::liftDeterministicConstraint(
  const RobustOptimizationProblemCanonicalizer & canonicalizer) const
{
  if (!hasInequalityConstraint())
    return Function();
  if (!canonicalizer.getOptimizeParameter())
    return getInequalityConstraint();
  const UnsignedInteger designDimension = getObjective().getInputDimension();
  const UnsignedInteger canonicalDimension = canonicalizer.getCanonicalProblem().getDimension();
  if (canonicalDimension == designDimension)
    return getInequalityConstraint();
  Matrix selection(designDimension, canonicalDimension);
  for (UnsignedInteger i = 0; i < designDimension; ++i)
    selection(i, i) = 1.0;
  const LinearFunction designProjection(Point(canonicalDimension),
                                        Point(designDimension),
                                        selection);
  return ComposedFunction(getInequalityConstraint(), designProjection);
}

/* Solver-ready canonical problem accessors */
RobustOptimizationProblem RBDOProblem::getCanonicalProblem() const
{
  return getCanonicalProblem(GetDefaultCanonicalizationOptions());
}

RobustOptimizationProblem RBDOProblem::getCanonicalProblem(
  const CanonicalizationOptions & options) const
{
  const RobustOptimizationProblemCanonicalizer canonicalizer(getCanonicalizer(options));
  const RobustOptimizationProblem canonicalProblem(canonicalizer.getCanonicalProblem());
  const Function chanceConstraint(canonicalProblem.getInequalityConstraint());
  Function inequalityConstraint(chanceConstraint);
  if (hasInequalityConstraint())
  {
    Collection<Function> components(2);
    components[0] = chanceConstraint;
    components[1] = liftDeterministicConstraint(canonicalizer);
    inequalityConstraint = AggregatedFunction(components);
  }
  RobustOptimizationProblem result;
  result.setObjective(canonicalProblem.getObjective());
  result.setBounds(canonicalProblem.getBounds());
  if (canonicalProblem.hasEqualityConstraint())
    result.setEqualityConstraint(canonicalProblem.getEqualityConstraint());
  result.setInequalityConstraint(inequalityConstraint);
  result.setVariablesType(canonicalProblem.getVariablesType());
  result.setMinimization(canonicalProblem.isMinimization(0), 0);
  return result;
}

/* Canonical joint chance measure accessors */
JointChanceMeasure RBDOProblem::getCanonicalJointChanceMeasure() const
{
  return getCanonicalJointChanceMeasure(GetDefaultCanonicalizationOptions());
}

JointChanceMeasure RBDOProblem::getCanonicalJointChanceMeasure(
  const CanonicalizationOptions & options) const
{
  const RobustOptimizationProblemCanonicalizer canonicalizer(getCanonicalizer(options));
  const MeasureEvaluation measure(canonicalizer.getCanonicalProblem().getReliabilityMeasure());
  const JointChanceMeasure * p_measure = dynamic_cast<const JointChanceMeasure *>(measure.getImplementation().get());
  if (!p_measure)
    throw InternalException(HERE) << "The canonical reliability measure is not a joint chance measure";
  return JointChanceMeasure(*p_measure);
}

/* Canonical deterministic inequality constraint accessors */
Function RBDOProblem::getCanonicalDeterministicConstraint() const
{
  return getCanonicalDeterministicConstraint(GetDefaultCanonicalizationOptions());
}

Function RBDOProblem::getCanonicalDeterministicConstraint(
  const CanonicalizationOptions & options) const
{
  if (!hasInequalityConstraint())
    return Function();
  const RobustOptimizationProblemCanonicalizer canonicalizer(getCanonicalizer(options));
  return liftDeterministicConstraint(canonicalizer);
}

/* Canonical starting point accessors */
Point RBDOProblem::getCanonicalStartingPoint() const
{
  return getCanonicalStartingPoint(GetDefaultCanonicalizationOptions());
}

Point RBDOProblem::getCanonicalStartingPoint(
  const CanonicalizationOptions & options) const
{
  return getCanonicalizer(options).getCanonicalStartingPoint();
}

/* String converter */
String RBDOProblem::__repr__() const
{
  OSS oss;
  oss << "class=" << RBDOProblem::GetClassName()
      << " objective=" << getObjective()
      << " jointChanceMeasure=" << jointChanceMeasure_
      << " inequalityConstraint=" << getInequalityConstraint()
      << " equalityConstraint=" << getEqualityConstraint();
  return oss;
}

/* Method save() stores the object through the StorageManager */
void RBDOProblem::save(Advocate & adv) const
{
  OptimizationProblemImplementation::save(adv);
  adv.saveAttribute("jointChanceMeasure_", jointChanceMeasure_);
}

/* Method load() reloads the object from the StorageManager */
void RBDOProblem::load(Advocate & adv)
{
  OptimizationProblemImplementation::load(adv);
  adv.loadAttribute("jointChanceMeasure_", jointChanceMeasure_);
}

} /* namespace OTROBOPT */
