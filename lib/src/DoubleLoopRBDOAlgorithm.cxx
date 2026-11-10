//                                               -*- C++ -*-
/**
 *  @brief Double-loop reliability-based design optimization
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
#include "otrobopt/DoubleLoopRBDOAlgorithm.hxx"

#include <openturns/AggregatedFunction.hxx>
#include <openturns/Cobyla.hxx>
#include <openturns/ComposedFunction.hxx>
#include <openturns/DistFunc.hxx>
#include <openturns/EvaluationImplementation.hxx>
#include <openturns/Exception.hxx>
#include <openturns/GradientImplementation.hxx>
#include <openturns/NearestPointProblem.hxx>
#include <openturns/NoHessian.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/ResourceMap.hxx>

#include <cmath>
#include <limits>
#include <memory>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(DoubleLoopRBDOAlgorithm)

static Factory<DoubleLoopRBDOAlgorithm> Factory_DoubleLoopRBDOAlgorithm;

namespace
{

const Scalar NaN = std::numeric_limits<Scalar>::quiet_NaN();

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

struct InnerReport
{
  Scalar constraint = 0.0;
  Scalar beta = NaN;
  Scalar performance = NaN;
  Point standardDesignPoint;
};

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
    const Scalar diagonal = betaTarget / std::sqrt(static_cast<Scalar>(dimension));
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
                   const char * analysis)
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

/* One solved reliability component */
struct ReliabilityReport
{
  InnerReport inner;
  UnsignedInteger component = 0;
};

/* Solve all the reliability components of a joint chance constraint */
Collection<ReliabilityReport> AnalyzeReliability(const RBDOProblem & problem,
                                                 const DoubleLoopRBDOAlgorithm::Method method,
                                                 const DoubleLoopRBDOAlgorithm::JointChanceTreatment treatment,
                                                 const OptimizationAlgorithm & innerSolver,
                                                 const Point & designPoint)
{
  const Function limitState(problem.getLimitStateFunction());
  const UnsignedInteger outputDimension = limitState.getOutputDimension();
  const ComparisonOperator op(problem.getComparisonOperator());
  const Scalar safeSign = GetSafeSign(op);
  const Scalar safeProbability = GetSafeProbability(op, problem.getTargetProbability());
  const Distribution distribution(problem.getDistribution());
  const Function inverseTransformation(distribution.getInverseIsoProbabilisticTransformation());
  const Function vectorFunction(BuildStandardLimitState(problem, designPoint, inverseTransformation));

  Collection<UnsignedInteger> outputIndices;
  Collection<Scalar> betaTargets;
  if ((treatment == DoubleLoopRBDOAlgorithm::BONFERRONI) && (outputDimension > 1))
  {
    const Scalar componentAlpha = 1.0 - (1.0 - safeProbability) / outputDimension;
    if (!(componentAlpha > 0.5) || !(componentAlpha < 1.0))
      throw InvalidArgumentException(HERE) << "The Bonferroni treatment requires a component safe probability in (0.5, 1), got "
                                           << componentAlpha << " for a joint safe probability " << safeProbability
                                           << " and " << outputDimension << " limit-state components. Use the exact treatment or a sample-based algorithm.";
    const Scalar betaTarget = DistFunc::qNormal(componentAlpha);
    for (UnsignedInteger i = 0; i < outputDimension; ++i)
    {
      outputIndices.add(i);
      betaTargets.add(betaTarget);
    }
  }
  else
  {
    outputIndices.add(outputDimension);
    betaTargets.add(DistFunc::qNormal(safeProbability));
  }

  Collection<ReliabilityReport> reports;
  for (UnsignedInteger i = 0; i < outputIndices.getSize(); ++i)
  {
    const Function function(BuildScalarizedFunction(vectorFunction, safeSign, outputIndices[i]));
    ReliabilityReport report;
    report.component = outputIndices[i];
    if (method == DoubleLoopRBDOAlgorithm::RIA)
      report.inner = SolveRIA(function, betaTargets[i], innerSolver, Point());
    else
      report.inner = SolvePMA(function, betaTargets[i], innerSolver, Point());
    reports.add(report);
  }
  return reports;
}

/* Shared state of the chance constraint evaluated by the outer solver */
class ChanceState
{
public:
  ChanceState(const RBDOProblem & problem,
              const DoubleLoopRBDOAlgorithm::Method method,
              const DoubleLoopRBDOAlgorithm::JointChanceTreatment treatment,
              const OptimizationAlgorithm & innerSolver)
    : problem_(problem)
    , method_(method)
    , treatment_(treatment)
    , innerSolver_(innerSolver)
    , limitState_(problem.getLimitStateFunction())
    , safeSign_(GetSafeSign(problem.getComparisonOperator()))
    , safeProbability_(GetSafeProbability(problem.getComparisonOperator(), problem.getTargetProbability()))
    , outputDimension_(limitState_.getOutputDimension())
    , inverseTransformation_(problem.getDistribution().getInverseIsoProbabilisticTransformation())
  {
    if ((treatment_ == DoubleLoopRBDOAlgorithm::BONFERRONI) && (outputDimension_ > 1))
    {
      const Scalar componentAlpha = 1.0 - (1.0 - safeProbability_) / outputDimension_;
      if (!(componentAlpha > 0.5) || !(componentAlpha < 1.0))
        throw InvalidArgumentException(HERE) << "The Bonferroni treatment requires a component safe probability in (0.5, 1), got "
                                             << componentAlpha << " for a joint safe probability " << safeProbability_
                                             << " and " << outputDimension_ << " limit-state components. Use the exact treatment or a sample-based algorithm.";
      const Scalar betaTarget = DistFunc::qNormal(componentAlpha);
      for (UnsignedInteger i = 0; i < outputDimension_; ++i)
      {
        outputIndices_.add(i);
        betaTargets_.add(betaTarget);
      }
    }
    else
    {
      outputIndices_.add(outputDimension_);
      betaTargets_.add(DistFunc::qNormal(safeProbability_));
    }
    warmStarts_ = Collection<Point>(outputIndices_.getSize());
  }

  UnsignedInteger getDesignDimension() const
  {
    return problem_.getObjective().getInputDimension();
  }

  Description getDesignDescription() const
  {
    return problem_.getObjective().getInputDescription();
  }

  Scalar evaluate(const Point & designPoint)
  {
    if (hasLastPoint_ && (designPoint.getDimension() == lastPoint_.getDimension()))
    {
      const Scalar distance = (designPoint - lastPoint_).norm();
      if (distance <= 1e-15)
        return lastConstraint_;
    }
    const Function vectorFunction(BuildStandardLimitState(problem_, designPoint, inverseTransformation_));
    Scalar worstConstraint = std::numeric_limits<Scalar>::infinity();
    for (UnsignedInteger i = 0; i < outputIndices_.getSize(); ++i)
    {
      const Function function(BuildScalarizedFunction(vectorFunction, safeSign_, outputIndices_[i]));
      InnerReport report;
      if (method_ == DoubleLoopRBDOAlgorithm::RIA)
        report = SolveRIA(function, betaTargets_[i], innerSolver_, warmStarts_[i]);
      else
        report = SolvePMA(function, betaTargets_[i], innerSolver_, warmStarts_[i]);
      warmStarts_[i] = report.standardDesignPoint;
      if (report.constraint < worstConstraint)
        worstConstraint = report.constraint;
    }
    lastPoint_ = designPoint;
    lastConstraint_ = worstConstraint;
    hasLastPoint_ = true;
    return worstConstraint;
  }

private:
  RBDOProblem problem_;
  DoubleLoopRBDOAlgorithm::Method method_;
  DoubleLoopRBDOAlgorithm::JointChanceTreatment treatment_;
  OptimizationAlgorithm innerSolver_;
  Function limitState_;
  Scalar safeSign_ = 1.0;
  Scalar safeProbability_ = 0.0;
  UnsignedInteger outputDimension_ = 0;
  Function inverseTransformation_;
  Collection<UnsignedInteger> outputIndices_;
  Collection<Scalar> betaTargets_;
  Collection<Point> warmStarts_;
  Point lastPoint_;
  Scalar lastConstraint_ = 0.0;
  Bool hasLastPoint_ = false;
};

/* Chance constraint function used by the outer solver */
class ChanceConstraintEvaluation
  : public EvaluationImplementation
{
public:
  explicit ChanceConstraintEvaluation(const std::shared_ptr<ChanceState> & state)
    : EvaluationImplementation()
    , state_(state)
  {
    // Nothing to do
  }

  ChanceConstraintEvaluation * clone() const override
  {
    return new ChanceConstraintEvaluation(state_);
  }

  Point operator()(const Point & designPoint) const override
  {
    return Point(1, state_->evaluate(designPoint));
  }

  UnsignedInteger getInputDimension() const override
  {
    return state_->getDesignDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return 1;
  }

  Description getInputDescription() const override
  {
    return state_->getDesignDescription();
  }

  Description getOutputDescription() const override
  {
    return Description(1, "chance");
  }

private:
  std::shared_ptr<ChanceState> state_;
};

/* Default inner reliability solver */
OptimizationAlgorithm BuildDefaultInnerSolver()
{
  Cobyla solver;
  solver.setMaximumIterationNumber(ResourceMap::GetAsUnsignedInteger("DoubleLoopRBDOAlgorithm-DefaultInnerMaximumIterationNumber"));
  solver.setMaximumConstraintError(ResourceMap::GetAsScalar("DoubleLoopRBDOAlgorithm-DefaultInnerMaximumConstraintError"));
  solver.setMaximumAbsoluteError(ResourceMap::GetAsScalar("DoubleLoopRBDOAlgorithm-DefaultInnerMaximumAbsoluteError"));
  return solver;
}

const char * MethodName(const DoubleLoopRBDOAlgorithm::Method method)
{
  return method == DoubleLoopRBDOAlgorithm::RIA ? "RIA" : "PMA";
}

const char * TreatmentName(const DoubleLoopRBDOAlgorithm::JointChanceTreatment treatment)
{
  return treatment == DoubleLoopRBDOAlgorithm::BONFERRONI ? "BONFERRONI" : "EXACT";
}

} /* anonymous namespace */

/* Default constructor */
DoubleLoopRBDOAlgorithm::DoubleLoopRBDOAlgorithm()
  : OptimizationAlgorithmImplementation()
  , solver_(Cobyla())
  , innerSolver_(BuildDefaultInnerSolver())
{
  // Nothing to do
}

/* Parameter constructor */
DoubleLoopRBDOAlgorithm::DoubleLoopRBDOAlgorithm(const RBDOProblem & problem,
                                                 const OptimizationAlgorithm & solver)
  : OptimizationAlgorithmImplementation()
  , solver_(solver)
  , innerSolver_(BuildDefaultInnerSolver())
{
  setRBDOProblem(problem);
}

/* Parameter constructor */
DoubleLoopRBDOAlgorithm::DoubleLoopRBDOAlgorithm(const RBDOProblem & problem,
                                                 const OptimizationAlgorithm & solver,
                                                 const OptimizationAlgorithm & innerSolver)
  : OptimizationAlgorithmImplementation()
  , solver_(solver)
  , innerSolver_(innerSolver)
{
  setRBDOProblem(problem);
}

/* Virtual constructor method */
DoubleLoopRBDOAlgorithm * DoubleLoopRBDOAlgorithm::clone() const
{
  return new DoubleLoopRBDOAlgorithm(*this);
}

/* Performs the actual computation */
void DoubleLoopRBDOAlgorithm::run()
{
  rbdoProblem_.check();
  const ComparisonOperator op(rbdoProblem_.getComparisonOperator());
  const Scalar safeProbability = GetSafeProbability(op, rbdoProblem_.getTargetProbability());
  if (!(safeProbability > 0.5) || !(safeProbability < 1.0))
    throw InvalidArgumentException(HERE) << "DoubleLoopRBDOAlgorithm requires a safe-event target probability in (0.5, 1), got "
                                         << safeProbability << " for the joint chance measure with target "
                                         << rbdoProblem_.getTargetProbability() << " and operator " << op
                                         << ". Use SampleAverageApproximationAlgorithm for other target probabilities.";
  // Validate the operator and the joint treatment before building the state
  (void) GetSafeSign(op);

  const std::shared_ptr<ChanceState> state(new ChanceState(rbdoProblem_,
                                                           method_,
                                                           jointChanceTreatment_,
                                                           innerSolver_));
  Function chanceFunction(Evaluation(new ChanceConstraintEvaluation(state)));

  OptimizationProblem outerProblem;
  outerProblem.setObjective(rbdoProblem_.getObjective());
  if (rbdoProblem_.getBounds().getDimension() != 0)
    outerProblem.setBounds(rbdoProblem_.getBounds());
  if (rbdoProblem_.getVariablesType().getSize() != 0)
    outerProblem.setVariablesType(rbdoProblem_.getVariablesType());
  AggregatedFunction::FunctionCollection inequalities;
  inequalities.add(chanceFunction);
  if (rbdoProblem_.hasInequalityConstraint())
    inequalities.add(rbdoProblem_.getInequalityConstraint());
  outerProblem.setInequalityConstraint(AggregatedFunction(inequalities));
  if (rbdoProblem_.hasEqualityConstraint())
    outerProblem.setEqualityConstraint(rbdoProblem_.getEqualityConstraint());
  outerProblem.setMinimization(rbdoProblem_.isMinimization(0), 0);

  OptimizationAlgorithm solver(solver_);
  solver.setProblem(outerProblem);
  solver.setStartingPoint(buildStartingPoint());
  solver.run();
  result_ = solver.getResult();

  const Point optimalPoint(result_.getOptimalPoint());
  if (optimalPoint.getDimension() != rbdoProblem_.getObjective().getInputDimension())
    throw InternalException(HERE) << "The outer optimization did not return a design point of dimension "
                                 << rbdoProblem_.getObjective().getInputDimension();

  // Essential report: the formulation selected by the user.
  const Collection<ReliabilityReport> essentialReports(AnalyzeReliability(rbdoProblem_,
                                                                          method_,
                                                                          jointChanceTreatment_,
                                                                          innerSolver_,
                                                                          optimalPoint));
  const ReliabilityReport * essentialWorst = nullptr;
  for (UnsignedInteger i = 0; i < essentialReports.getSize(); ++i)
    if ((essentialWorst == nullptr) || (essentialReports[i].inner.constraint < essentialWorst->inner.constraint))
      essentialWorst = &essentialReports[i];

  if (method_ == RIA)
    reliabilityIndex_ = essentialWorst->inner.beta;
  else
    performanceMeasure_ = essentialWorst->inner.performance;

  // Optional report: the other formulation, used for diagnostics.
  try
  {
    const Method otherMethod = (method_ == RIA) ? PMA : RIA;
    const Collection<ReliabilityReport> otherReports(AnalyzeReliability(rbdoProblem_,
                                                                        otherMethod,
                                                                        jointChanceTreatment_,
                                                                        innerSolver_,
                                                                        optimalPoint));
    const ReliabilityReport * otherWorst = nullptr;
    for (UnsignedInteger i = 0; i < otherReports.getSize(); ++i)
      if ((otherWorst == nullptr) || (otherReports[i].inner.constraint < otherWorst->inner.constraint))
        otherWorst = &otherReports[i];
    if (otherMethod == RIA)
      reliabilityIndex_ = otherWorst->inner.beta;
    else
      performanceMeasure_ = otherWorst->inner.performance;
  }
  catch (const Exception &)
  {
    // The diagnostic analysis is not essential to the result.
  }

  standardSpaceDesignPoint_ = essentialWorst->inner.standardDesignPoint;
  const Distribution distribution(rbdoProblem_.getDistribution());
  physicalSpaceDesignPoint_ = distribution.getInverseIsoProbabilisticTransformation()(standardSpaceDesignPoint_);
  if (std::isfinite(reliabilityIndex_))
    approximateProbability_ = DistFunc::pNormal(reliabilityIndex_);
  else
    approximateProbability_ = NaN;
  hasRun_ = true;
}

/* Problem accessor */
void DoubleLoopRBDOAlgorithm::setProblem(const OptimizationProblem & problem)
{
  const RBDOProblem * p_problem = dynamic_cast<const RBDOProblem *>(problem.getImplementation().get());
  if (p_problem == nullptr)
    throw InvalidArgumentException(HERE) << "DoubleLoopRBDOAlgorithm expects an RBDOProblem, got "
                                         << problem.getImplementation()->getClassName();
  OptimizationAlgorithmImplementation::setProblem(problem);
  rbdoProblem_ = *p_problem;
  hasRun_ = false;
}

void DoubleLoopRBDOAlgorithm::setRBDOProblem(const RBDOProblem & problem)
{
  setProblem(problem);
}

RBDOProblem DoubleLoopRBDOAlgorithm::getRBDOProblem() const
{
  return rbdoProblem_;
}

/* Outer optimization solver accessors */
void DoubleLoopRBDOAlgorithm::setOptimizationAlgorithm(const OptimizationAlgorithm & solver)
{
  solver_ = solver;
  hasRun_ = false;
}

OptimizationAlgorithm DoubleLoopRBDOAlgorithm::getOptimizationAlgorithm() const
{
  return solver_;
}

/* Inner reliability solver accessors */
void DoubleLoopRBDOAlgorithm::setInnerOptimizationAlgorithm(const OptimizationAlgorithm & solver)
{
  innerSolver_ = solver;
  hasRun_ = false;
}

OptimizationAlgorithm DoubleLoopRBDOAlgorithm::getInnerOptimizationAlgorithm() const
{
  return innerSolver_;
}

/* Inner formulation accessors */
void DoubleLoopRBDOAlgorithm::setMethod(const Method method)
{
  if ((method != RIA) && (method != PMA))
    throw InvalidArgumentException(HERE) << "The inner reliability method must be RIA or PMA";
  method_ = method;
  hasRun_ = false;
}

DoubleLoopRBDOAlgorithm::Method DoubleLoopRBDOAlgorithm::getMethod() const
{
  return method_;
}

/* Joint chance treatment accessors */
void DoubleLoopRBDOAlgorithm::setJointChanceTreatment(const JointChanceTreatment treatment)
{
  if ((treatment != EXACT) && (treatment != BONFERRONI))
    throw InvalidArgumentException(HERE) << "The joint chance treatment must be EXACT or BONFERRONI";
  jointChanceTreatment_ = treatment;
  hasRun_ = false;
}

DoubleLoopRBDOAlgorithm::JointChanceTreatment DoubleLoopRBDOAlgorithm::getJointChanceTreatment() const
{
  return jointChanceTreatment_;
}

/* Target reliability index defined by the joint chance constraint */
Scalar DoubleLoopRBDOAlgorithm::getTargetReliabilityIndex() const
{
  if (!rbdoProblem_.hasJointChanceMeasure())
    throw InvalidArgumentException(HERE) << "No reliability-based design optimization problem is defined";
  const Scalar alpha = rbdoProblem_.getTargetProbability();
  const UnsignedInteger outputDimension = rbdoProblem_.getLimitStateFunction().getOutputDimension();
  Scalar targetAlpha = GetSafeProbability(rbdoProblem_.getComparisonOperator(), alpha);
  if ((jointChanceTreatment_ == BONFERRONI) && (outputDimension > 1))
    targetAlpha = 1.0 - (1.0 - targetAlpha) / outputDimension;
  if (!(targetAlpha > 0.5) || !(targetAlpha < 1.0))
    throw InvalidArgumentException(HERE) << "The safe-event probability associated with the selected joint chance treatment is "
                                         << targetAlpha << ", which is outside (0.5, 1)";
  return DistFunc::qNormal(targetAlpha);
}

/* Signed reliability index at the solution */
Scalar DoubleLoopRBDOAlgorithm::getReliabilityIndex() const
{
  if (!hasRun_)
    throw NotDefinedException(HERE) << "run() must be called before getReliabilityIndex()";
  return reliabilityIndex_;
}

/* Performance measure at the solution */
Scalar DoubleLoopRBDOAlgorithm::getPerformanceMeasure() const
{
  if (!hasRun_)
    throw NotDefinedException(HERE) << "run() must be called before getPerformanceMeasure()";
  return performanceMeasure_;
}

/* Standard-space design point at the solution */
Point DoubleLoopRBDOAlgorithm::getStandardSpaceDesignPoint() const
{
  if (!hasRun_)
    throw NotDefinedException(HERE) << "run() must be called before getStandardSpaceDesignPoint()";
  return standardSpaceDesignPoint_;
}

/* Physical-space design point at the solution */
Point DoubleLoopRBDOAlgorithm::getPhysicalSpaceDesignPoint() const
{
  if (!hasRun_)
    throw NotDefinedException(HERE) << "run() must be called before getPhysicalSpaceDesignPoint()";
  return physicalSpaceDesignPoint_;
}

/* FORM approximation of the safe probability at the solution */
Scalar DoubleLoopRBDOAlgorithm::getApproximateProbability() const
{
  if (!hasRun_)
    throw NotDefinedException(HERE) << "run() must be called before getApproximateProbability()";
  return approximateProbability_;
}

/* Check whether this problem can be solved by this solver */
void DoubleLoopRBDOAlgorithm::checkProblem(const OptimizationProblem & problem) const
{
  const RBDOProblem * p_problem = dynamic_cast<const RBDOProblem *>(problem.getImplementation().get());
  if (p_problem == nullptr)
    throw InvalidArgumentException(HERE) << "DoubleLoopRBDOAlgorithm expects an RBDOProblem, got "
                                         << problem.getImplementation()->getClassName();
  p_problem->check();
  const ComparisonOperator op(p_problem->getComparisonOperator());
  const Scalar safeProbability = GetSafeProbability(op, p_problem->getTargetProbability());
  if (!(safeProbability > 0.5) || !(safeProbability < 1.0))
    throw InvalidArgumentException(HERE) << "DoubleLoopRBDOAlgorithm requires a safe-event target probability in (0.5, 1), got "
                                         << safeProbability << " for the joint chance measure with target "
                                         << p_problem->getTargetProbability() << " and operator " << op
                                         << ". Use SampleAverageApproximationAlgorithm for other target probabilities.";
  (void) GetSafeSign(op);
}

/* Build a starting point for the outer optimization */
Point DoubleLoopRBDOAlgorithm::buildStartingPoint() const
{
  const UnsignedInteger dimension = rbdoProblem_.getObjective().getInputDimension();
  const Point startingPoint(getStartingPoint());
  if (startingPoint.getDimension() != 0)
  {
    if (startingPoint.getDimension() != dimension)
      throw InvalidDimensionException(HERE) << "The starting point dimension (" << startingPoint.getDimension()
                                            << ") must match the design dimension (" << dimension << ")";
    return startingPoint;
  }
  try
  {
    const Point canonicalStartingPoint(rbdoProblem_.getCanonicalStartingPoint());
    if ((canonicalStartingPoint.getDimension() == dimension) && IsFinitePoint(canonicalStartingPoint))
      return canonicalStartingPoint;
  }
  catch (const Exception &)
  {
    // Fall back to the bounds or to the origin
  }
  if (rbdoProblem_.getBounds().getDimension() == dimension)
  {
    const Interval bounds(rbdoProblem_.getBounds());
    const Point lower(bounds.getLowerBound());
    const Point upper(bounds.getUpperBound());
    Point point(dimension);
    for (UnsignedInteger i = 0; i < dimension; ++i)
    {
      if (std::isfinite(lower[i]) && std::isfinite(upper[i]))
        point[i] = 0.5 * (lower[i] + upper[i]);
      else if (std::isfinite(lower[i]))
        point[i] = lower[i];
      else if (std::isfinite(upper[i]))
        point[i] = upper[i];
      else
        point[i] = 0.0;
    }
    return point;
  }
  return Point(dimension);
}

Bool DoubleLoopRBDOAlgorithm::hasRun() const
{
  return hasRun_;
}

/* String converter */
String DoubleLoopRBDOAlgorithm::__repr__() const
{
  OSS oss;
  oss << "class=" << DoubleLoopRBDOAlgorithm::GetClassName()
      << " method=" << MethodName(method_)
      << " jointChanceTreatment=" << TreatmentName(jointChanceTreatment_)
      << " problem=" << rbdoProblem_
      << " solver=" << solver_
      << " innerSolver=" << innerSolver_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void DoubleLoopRBDOAlgorithm::save(Advocate & adv) const
{
  OptimizationAlgorithmImplementation::save(adv);
  adv.saveAttribute("rbdoProblem_", rbdoProblem_);
  adv.saveAttribute("method_", static_cast<UnsignedInteger>(method_));
  adv.saveAttribute("jointChanceTreatment_", static_cast<UnsignedInteger>(jointChanceTreatment_));
  adv.saveAttribute("solver_", solver_);
  adv.saveAttribute("innerSolver_", innerSolver_);
  adv.saveAttribute("reliabilityIndex_", reliabilityIndex_);
  adv.saveAttribute("performanceMeasure_", performanceMeasure_);
  adv.saveAttribute("approximateProbability_", approximateProbability_);
  adv.saveAttribute("standardSpaceDesignPoint_", standardSpaceDesignPoint_);
  adv.saveAttribute("physicalSpaceDesignPoint_", physicalSpaceDesignPoint_);
  adv.saveAttribute("hasRun_", hasRun_);
}

/* Method load() reloads the object from the StorageManager */
void DoubleLoopRBDOAlgorithm::load(Advocate & adv)
{
  OptimizationAlgorithmImplementation::load(adv);
  adv.loadAttribute("rbdoProblem_", rbdoProblem_);
  UnsignedInteger method = 0;
  adv.loadAttribute("method_", method);
  if (method > 1)
    throw InvalidArgumentException(HERE) << "Invalid saved inner reliability method: " << method;
  method_ = static_cast<Method>(method);
  UnsignedInteger treatment = 0;
  adv.loadAttribute("jointChanceTreatment_", treatment);
  if (treatment > 1)
    throw InvalidArgumentException(HERE) << "Invalid saved joint chance treatment: " << treatment;
  jointChanceTreatment_ = static_cast<JointChanceTreatment>(treatment);
  adv.loadAttribute("solver_", solver_);
  adv.loadAttribute("innerSolver_", innerSolver_);
  adv.loadAttribute("reliabilityIndex_", reliabilityIndex_);
  adv.loadAttribute("performanceMeasure_", performanceMeasure_);
  adv.loadAttribute("approximateProbability_", approximateProbability_);
  adv.loadAttribute("standardSpaceDesignPoint_", standardSpaceDesignPoint_);
  adv.loadAttribute("physicalSpaceDesignPoint_", physicalSpaceDesignPoint_);
  adv.loadAttribute("hasRun_", hasRun_);
}

} /* namespace OTROBOPT */
