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
#include "otrobopt/RBDOAlgorithmTools.hxx"

#include <openturns/Cobyla.hxx>
#include <openturns/ComposedFunction.hxx>
#include <openturns/EvaluationImplementation.hxx>
#include <openturns/Exception.hxx>
#include <openturns/GradientImplementation.hxx>
#include <openturns/NearestPointProblem.hxx>
#include <openturns/NoHessian.hxx>
#include <openturns/ResourceMap.hxx>

#include <cmath>
#include <limits>

using namespace OT;

namespace OTROBOPT
{

namespace RBDOAlgorithmTools
{

/* Quiet NaN sentinel */
const Scalar NaN = std::numeric_limits<Scalar>::quiet_NaN();

namespace
{

/* Frozen limit-state function: theta -> G(x, theta) */
class FrozenLimitStateEvaluation
  : public EvaluationImplementation
{
public:
  FrozenLimitStateEvaluation(const Function & function,
                             const Point & designPoint)
    : EvaluationImplementation()
    , function_(function)
    , designPoint_(designPoint)
  {
    // Nothing to do
  }

  FrozenLimitStateEvaluation * clone() const override
  {
    return new FrozenLimitStateEvaluation(*this);
  }

  Point operator()(const Point & theta) const override
  {
    Function function(function_);
    function.setParameter(theta);
    return function(designPoint_);
  }

  UnsignedInteger getInputDimension() const override
  {
    return function_.getParameterDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return function_.getOutputDimension();
  }

  Description getInputDescription() const override
  {
    return function_.getParameterDescription();
  }

  Description getOutputDescription() const override
  {
    return function_.getOutputDescription();
  }

private:
  Function function_;
  Point designPoint_;
};

/* Gradient of the frozen limit-state function */
class FrozenLimitStateGradient
  : public GradientImplementation
{
public:
  FrozenLimitStateGradient(const Function & function,
                           const Point & designPoint)
    : GradientImplementation()
    , function_(function)
    , designPoint_(designPoint)
  {
    // Nothing to do
  }

  FrozenLimitStateGradient * clone() const override
  {
    return new FrozenLimitStateGradient(*this);
  }

  Matrix gradient(const Point & theta) const override
  {
    Function function(function_);
    function.setParameter(theta);
    const Matrix parameterGradient(function.parameterGradient(designPoint_));
    const UnsignedInteger outputDimension = parameterGradient.getNbColumns();
    const UnsignedInteger inputDimension = parameterGradient.getNbRows();
    Matrix result(outputDimension, inputDimension);
    for (UnsignedInteger i = 0; i < inputDimension; ++i)
      for (UnsignedInteger j = 0; j < outputDimension; ++j)
        result(j, i) = parameterGradient(i, j);
    return result;
  }

  UnsignedInteger getInputDimension() const override
  {
    return function_.getParameterDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return function_.getOutputDimension();
  }

private:
  Function function_;
  Point designPoint_;
};

/* Scalarization of a vector limit state in the standard space */
class ScalarizedLimitStateEvaluation
  : public EvaluationImplementation
{
public:
  /** outputIndex equal to the output dimension means the exact joint event */
  ScalarizedLimitStateEvaluation(const Function & function,
                                 const Scalar safeSign,
                                 const UnsignedInteger outputIndex)
    : EvaluationImplementation()
    , function_(function)
    , safeSign_(safeSign)
    , outputIndex_(outputIndex)
  {
    // Nothing to do
  }

  ScalarizedLimitStateEvaluation * clone() const override
  {
    return new ScalarizedLimitStateEvaluation(*this);
  }

  Point operator()(const Point & point) const override
  {
    const Point value(function_(point));
    if (outputIndex_ < value.getDimension())
      return Point(1, safeSign_ * value[outputIndex_]);
    // Exact joint event.  The measure integrates {G >= 0}, so for a Greater
    // operator the safe event is the intersection {min G >= 0}, while for a
    // Less operator it is the complementary union {min G < 0}, i.e.
    // {max(-G) >= 0} = {-min G >= 0}.
    Scalar result = value[0];
    for (UnsignedInteger i = 1; i < value.getDimension(); ++i)
      result = std::min(result, value[i]);
    return Point(1, safeSign_ * result);
  }

  UnsignedInteger getInputDimension() const override
  {
    return function_.getInputDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return 1;
  }

  Description getInputDescription() const override
  {
    return function_.getInputDescription();
  }

  Description getOutputDescription() const override
  {
    return Description(1, "G");
  }

private:
  Function function_;
  Scalar safeSign_ = 1.0;
  UnsignedInteger outputIndex_ = 0;
};

/* Gradient of the scalarized limit state */
class ScalarizedLimitStateGradient
  : public GradientImplementation
{
public:
  ScalarizedLimitStateGradient(const Function & function,
                               const Scalar safeSign,
                               const UnsignedInteger outputIndex)
    : GradientImplementation()
    , function_(function)
    , safeSign_(safeSign)
    , outputIndex_(outputIndex)
  {
    // Nothing to do
  }

  ScalarizedLimitStateGradient * clone() const override
  {
    return new ScalarizedLimitStateGradient(*this);
  }

  Matrix gradient(const Point & point) const override
  {
    const Point value(function_(point));
    const Matrix fullGradient(function_.gradient(point));
    const UnsignedInteger dimension = fullGradient.getNbColumns();
    Matrix result(1, dimension);
    if (outputIndex_ < value.getDimension())
    {
      for (UnsignedInteger j = 0; j < dimension; ++j)
        result(0, j) = safeSign_ * fullGradient(outputIndex_, j);
      return result;
    }
    UnsignedInteger activeIndex = 0;
    Scalar activeValue = value[0];
    for (UnsignedInteger i = 1; i < value.getDimension(); ++i)
    {
      const Scalar candidate = value[i];
      if (candidate < activeValue)
      {
        activeValue = candidate;
        activeIndex = i;
      }
    }
    for (UnsignedInteger j = 0; j < dimension; ++j)
      result(0, j) = safeSign_ * fullGradient(activeIndex, j);
    return result;
  }

  UnsignedInteger getInputDimension() const override
  {
    return function_.getInputDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return 1;
  }

private:
  Function function_;
  Scalar safeSign_ = 1.0;
  UnsignedInteger outputIndex_ = 0;
};

/* Sphere constraint beta^2 - ||u||^2 = 0 */
class SphereEvaluation
  : public EvaluationImplementation
{
public:
  SphereEvaluation(const UnsignedInteger dimension,
                   const Scalar radius)
    : EvaluationImplementation()
    , dimension_(dimension)
    , radius_(radius)
  {
    // Nothing to do
  }

  SphereEvaluation * clone() const override
  {
    return new SphereEvaluation(*this);
  }

  Point operator()(const Point & point) const override
  {
    return Point(1, radius_ * radius_ - point.normSquare());
  }

  UnsignedInteger getInputDimension() const override
  {
    return dimension_;
  }

  UnsignedInteger getOutputDimension() const override
  {
    return 1;
  }

private:
  UnsignedInteger dimension_ = 1;
  Scalar radius_ = 1.0;
};

/* The sphere constraint has a dimension-independent gradient.  The input
   dimension of the gradient is set when the function is built. */
class SphereGradient
  : public GradientImplementation
{
public:
  SphereGradient(const UnsignedInteger dimension,
                 const Scalar radius)
    : GradientImplementation()
    , dimension_(dimension)
    , radius_(radius)
  {
    // Nothing to do
  }

  SphereGradient * clone() const override
  {
    return new SphereGradient(*this);
  }

  Matrix gradient(const Point & point) const override
  {
    Matrix result(1, dimension_);
    for (UnsignedInteger i = 0; i < dimension_; ++i)
      result(0, i) = -2.0 * point[i];
    return result;
  }

  UnsignedInteger getInputDimension() const override
  {
    return dimension_;
  }

  UnsignedInteger getOutputDimension() const override
  {
    return 1;
  }

private:
  UnsignedInteger dimension_ = 1;
  Scalar radius_ = 1.0;
};

} /* anonymous namespace */

/* Whether a point has only finite coordinates */
Bool IsFinitePoint(const Point & point)
{
  for (UnsignedInteger i = 0; i < point.getDimension(); ++i)
    if (!std::isfinite(point[i]))
      return false;
  return true;
}

/* Safe event sign: +1 for Greater* and -1 for Less* operators */
Scalar GetSafeSign(const ComparisonOperator & op)
{
  const Bool greater = op(2.0, 1.0) && !op(1.0, 2.0);
  const Bool less = op(1.0, 2.0) && !op(2.0, 1.0);
  if (greater == less)
    throw InvalidArgumentException(HERE) << "The joint chance constraint operator must be a Greater or a Less comparison, got "
                                         << op;
  return greater ? 1.0 : -1.0;
}

/* Target probability of the safe event.  The measure always integrates the
   event {G >= 0}: for a Greater operator the safe event is {G >= 0} itself,
   whereas for a Less operator it is the complementary event {G < 0}. */
Scalar GetSafeProbability(const ComparisonOperator & op,
                          const Scalar alpha)
{
  return GetSafeSign(op) > 0.0 ? alpha : 1.0 - alpha;
}

/* Build the standard-space vector limit state for a given design */
Function BuildStandardLimitState(const RBDOProblem & problem,
                                 const Point & designPoint,
                                 const Function & inverseTransformation)
{
  const Function limitState(problem.getLimitStateFunction());
  const Function frozen(Evaluation(new FrozenLimitStateEvaluation(limitState, designPoint)),
                        Gradient(new FrozenLimitStateGradient(limitState, designPoint)),
                        Hessian(new NoHessian()));
  return ComposedFunction(frozen, inverseTransformation);
}

/* Build the standard-space scalarized function for one component or for the
   exact joint event. */
Function BuildScalarizedFunction(const Function & vectorFunction,
                                 const Scalar safeSign,
                                 const UnsignedInteger outputIndex)
{
  return Function(Evaluation(new ScalarizedLimitStateEvaluation(vectorFunction, safeSign, outputIndex)),
                  Gradient(new ScalarizedLimitStateGradient(vectorFunction, safeSign, outputIndex)),
                  Hessian(new NoHessian()));
}

/* Evaluate a scalarized limit state at the origin of the standard space */
Scalar OriginValue(const Function & function)
{
  const Scalar value = function(Point(function.getInputDimension()))[0];
  if (!std::isfinite(value))
    throw InternalException(HERE) << "The limit-state function returned a non-finite value at the origin of the standard space: "
                                 << value;
  return value;
}

/* Build the starting points of an inner reliability analysis. */
Collection<Point> BuildStartingPoints(const Function & function,
                                      const Scalar betaTarget,
                                      const Point & warmStart,
                                      const Bool forPMA)
{
  const UnsignedInteger dimension = function.getInputDimension();
  Collection<Point> starts;
  const Scalar tolerance = 1e-12;
  const auto add = [&](const Point & point)
  {
    if (point.getDimension() != dimension || !IsFinitePoint(point))
      return;
    for (UnsignedInteger i = 0; i < starts.getSize(); ++i)
      if ((starts[i] - point).norm() <= tolerance)
        return;
    starts.add(point);
  };
  if (warmStart.getDimension() == dimension && IsFinitePoint(warmStart))
  {
    if (forPMA)
    {
      const Scalar norm = warmStart.norm();
      if (norm > tolerance)
      {
        Point point(dimension);
        for (UnsignedInteger i = 0; i < dimension; ++i)
          point[i] = warmStart[i] * betaTarget / norm;
        add(point);
      }
    }
    else
      add(warmStart);
  }

  const Point origin(dimension);
  const Scalar originValue = function(origin)[0];
  if (!std::isfinite(originValue))
    throw InternalException(HERE) << "The limit-state function returned a non-finite value at the origin of the standard space: "
                                 << originValue;
  try
  {
    const Matrix gradient(function.gradient(origin));
    if ((gradient.getNbRows() == 1) && (gradient.getNbColumns() == dimension))
    {
      Scalar normSquare = 0.0;
      for (UnsignedInteger i = 0; i < dimension; ++i)
        normSquare += gradient(0, i) * gradient(0, i);
      const Scalar norm = std::sqrt(normSquare);
      if (std::isfinite(norm) && (norm > tolerance))
      {
        const Scalar sign = forPMA ? -1.0 : ((originValue >= 0.0) ? -1.0 : 1.0);
        Point point(dimension);
        for (UnsignedInteger i = 0; i < dimension; ++i)
          point[i] = sign * betaTarget * gradient(0, i) / norm;
        add(point);
      }
    }
  }
  catch (const Exception &)
  {
    // The gradient is optional: fall back to the default starting point
  }

  if (dimension > 1)
  {
    // Nonsmooth scalarizations have their optimum at a corner; a diagonal
    // start lets the inner solver reach it from both sides.
    const Scalar diagonal = betaTarget / std::sqrt(1.0 * dimension);
    Point point(dimension);
    for (UnsignedInteger i = 0; i < dimension; ++i)
      point[i] = diagonal;
    add(point);
    for (UnsignedInteger i = 0; i < dimension; ++i)
      point[i] = -diagonal;
    add(point);
  }

  if (starts.getSize() == 0)
  {
    Point point(dimension);
    point[0] = betaTarget;
    add(point);
  }
  if (dimension == 1)
  {
    Point point(1);
    point[0] = -betaTarget;
    add(point);
    point[0] = betaTarget;
    add(point);
  }
  return starts;
}

/* Check the residual of an inner reliability analysis */
void CheckResidual(const OptimizationAlgorithm & solver,
                   const Scalar residual,
                   const String & analysis)
{
  const Scalar tolerance = std::max(100.0 * solver.getMaximumConstraintError(), 1e-6);
  if (!(residual <= tolerance))
    throw InternalException(HERE) << "The inner " << analysis << " reliability analysis did not reach the limit state: "
                                 << "constraint error=" << residual << " is larger than tolerance=" << tolerance;
}

/* RIA inner analysis */
InnerReport SolveRIA(const Function & function,
                     const Scalar betaTarget,
                     const OptimizationAlgorithm & innerSolver,
                     const Point & warmStart)
{
  const UnsignedInteger dimension = function.getInputDimension();
  const Scalar originValue = OriginValue(function);
  const Scalar boundaryTolerance = std::max(10.0 * innerSolver.getMaximumConstraintError(), 1e-10);
  if (std::abs(originValue) <= boundaryTolerance)
  {
    InnerReport report;
    report.constraint = -betaTarget;
    report.beta = 0.0;
    report.standardDesignPoint = Point(dimension);
    return report;
  }
  const Collection<Point> starts(BuildStartingPoints(function, betaTarget, warmStart, false));
  const Scalar tolerance = std::max(100.0 * innerSolver.getMaximumConstraintError(), 1e-6);
  Scalar bestDistance = std::numeric_limits<Scalar>::infinity();
  Scalar bestResidual = std::numeric_limits<Scalar>::infinity();
  Point bestPoint;
  Scalar bestFeasibleDistance = std::numeric_limits<Scalar>::infinity();
  Scalar bestFeasibleResidual = std::numeric_limits<Scalar>::infinity();
  Point bestFeasiblePoint;
  for (UnsignedInteger i = 0; i < starts.getSize(); ++i)
  {
    NearestPointProblem problem(function, 0.0);
    OptimizationAlgorithm solver(innerSolver);
    solver.setProblem(problem);
    solver.setStartingPoint(starts[i]);
    solver.run();
    const OptimizationResult result(solver.getResult());
    const Point point(result.getOptimalPoint());
    if (point.getDimension() != dimension)
      continue;
    const Scalar distance = point.norm();
    const Scalar residual = result.getConstraintError();
    if (std::isfinite(distance) && (distance < bestDistance))
    {
      bestDistance = distance;
      bestResidual = residual;
      bestPoint = point;
    }
    if (std::isfinite(distance) && (residual <= tolerance) && (distance < bestFeasibleDistance))
    {
      bestFeasibleDistance = distance;
      bestFeasibleResidual = residual;
      bestFeasiblePoint = point;
    }
  }
  if (bestFeasiblePoint.getDimension() == dimension)
  {
    bestDistance = bestFeasibleDistance;
    bestResidual = bestFeasibleResidual;
    bestPoint = bestFeasiblePoint;
  }
  if (bestPoint.getDimension() != dimension)
    throw InternalException(HERE) << "The inner RIA reliability analysis did not return a design point";
  CheckResidual(innerSolver, bestResidual, "RIA");
  Scalar beta = bestDistance;
  if (originValue < 0.0)
    beta = -beta;
  InnerReport report;
  report.constraint = beta - betaTarget;
  report.beta = beta;
  report.standardDesignPoint = bestPoint;
  return report;
}

/* PMA inner analysis */
InnerReport SolvePMA(const Function & function,
                     const Scalar betaTarget,
                     const OptimizationAlgorithm & innerSolver,
                     const Point & warmStart)
{
  const UnsignedInteger dimension = function.getInputDimension();
  if (dimension == 1)
  {
    Point lower(1, -betaTarget);
    Point upper(1, betaTarget);
    const Scalar lowerValue = function(lower)[0];
    const Scalar upperValue = function(upper)[0];
    if (!std::isfinite(lowerValue) || !std::isfinite(upperValue))
      throw InternalException(HERE) << "The limit-state function returned a non-finite value on the target sphere";
    InnerReport report;
    if (lowerValue <= upperValue)
    {
      report.constraint = lowerValue;
      report.performance = lowerValue;
      report.standardDesignPoint = lower;
    }
    else
    {
      report.constraint = upperValue;
      report.performance = upperValue;
      report.standardDesignPoint = upper;
    }
    return report;
  }

  const Scalar originValue = OriginValue(function);
  (void) originValue;
  const Collection<Point> starts(BuildStartingPoints(function, betaTarget, warmStart, true));
  const Scalar tolerance = std::max(100.0 * innerSolver.getMaximumConstraintError(), 1e-6);
  Scalar bestValue = std::numeric_limits<Scalar>::infinity();
  Scalar bestResidual = std::numeric_limits<Scalar>::infinity();
  Point bestPoint;
  Scalar bestFeasibleValue = std::numeric_limits<Scalar>::infinity();
  Scalar bestFeasibleResidual = std::numeric_limits<Scalar>::infinity();
  Point bestFeasiblePoint;
  for (UnsignedInteger i = 0; i < starts.getSize(); ++i)
  {
    OptimizationProblem problem(function);
    problem.setEqualityConstraint(Function(Evaluation(new SphereEvaluation(dimension, betaTarget)),
                                           Gradient(new SphereGradient(dimension, betaTarget)),
                                           Hessian(new NoHessian())));
    OptimizationAlgorithm solver(innerSolver);
    solver.setProblem(problem);
    solver.setStartingPoint(starts[i]);
    solver.run();
    const OptimizationResult result(solver.getResult());
    const Point point(result.getOptimalPoint());
    if (point.getDimension() != dimension)
      continue;
    const Scalar value = function(point)[0];
    const Scalar residual = result.getConstraintError();
    if (std::isfinite(value) && (value < bestValue))
    {
      bestValue = value;
      bestResidual = residual;
      bestPoint = point;
    }
    if (std::isfinite(value) && (residual <= tolerance) && (value < bestFeasibleValue))
    {
      bestFeasibleValue = value;
      bestFeasibleResidual = residual;
      bestFeasiblePoint = point;
    }
  }
  if (bestFeasiblePoint.getDimension() == dimension)
  {
    bestValue = bestFeasibleValue;
    bestResidual = bestFeasibleResidual;
    bestPoint = bestFeasiblePoint;
  }
  if (bestPoint.getDimension() != dimension)
    throw InternalException(HERE) << "The inner PMA reliability analysis did not return a design point";
  CheckResidual(innerSolver, bestResidual, "PMA");
  InnerReport report;
  report.constraint = bestValue;
  report.performance = bestValue;
  report.standardDesignPoint = bestPoint;
  return report;
}

/* Default inner reliability solver */
OptimizationAlgorithm BuildDefaultInnerSolver(const String & keyPrefix)
{
  Cobyla solver;
  solver.setMaximumIterationNumber(ResourceMap::GetAsUnsignedInteger(keyPrefix + "-DefaultInnerMaximumIterationNumber"));
  solver.setMaximumConstraintError(ResourceMap::GetAsScalar(keyPrefix + "-DefaultInnerMaximumConstraintError"));
  solver.setMaximumAbsoluteError(ResourceMap::GetAsScalar(keyPrefix + "-DefaultInnerMaximumAbsoluteError"));
  return solver;
}

} /* namespace RBDOAlgorithmTools */

} /* namespace OTROBOPT */
