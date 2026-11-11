//                                               -*- C++ -*-
/**
 *  @brief Sequential optimization and reliability assessment
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
#include "otrobopt/SORARobustOptimizationAlgorithm.hxx"
#include "otrobopt/RBDOAlgorithmTools.hxx"

#include <openturns/AggregatedFunction.hxx>
#include <openturns/Cobyla.hxx>
#include <openturns/Description.hxx>
#include <openturns/DistFunc.hxx>
#include <openturns/EvaluationImplementation.hxx>
#include <openturns/Exception.hxx>
#include <openturns/GradientImplementation.hxx>
#include <openturns/Interval.hxx>
#include <openturns/NoHessian.hxx>
#include <openturns/OptimizationProblem.hxx>
#include <openturns/OptimizationResult.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/ResourceMap.hxx>

#include <cmath>
#include <limits>
#include <memory>

using namespace OT;

namespace OTROBOPT
{

using namespace RBDOAlgorithmTools;

CLASSNAMEINIT(SORARobustOptimizationAlgorithm)

static Factory<SORARobustOptimizationAlgorithm> Factory_SORARobustOptimizationAlgorithm;

namespace
{

const Scalar NaN = std::numeric_limits<Scalar>::quiet_NaN();

/* One solved reliability component */
struct ReliabilityReport
{
  InnerReport inner;
  UnsignedInteger component = 0;
  Point physicalDesignPoint;
};

/* Evaluation of the exact shifted constraint sigma min_j g_j(x; theta*) */
class ExactShiftedEvaluation
  : public EvaluationImplementation
{
public:
  ExactShiftedEvaluation(const Function & limitState,
                         const Point & parameter,
                         const Scalar safeSign)
    : EvaluationImplementation()
    , limitState_(limitState)
    , parameter_(parameter)
    , safeSign_(safeSign)
  {
    if (parameter_.getDimension() != limitState_.getParameterDimension())
      throw InvalidDimensionException(HERE) << "The physical shift parameter dimension ("
                                            << parameter_.getDimension()
                                            << ") must match the limit-state parameter dimension ("
                                            << limitState_.getParameterDimension() << ")";
  }

  ExactShiftedEvaluation * clone() const override
  {
    return new ExactShiftedEvaluation(*this);
  }

  Point operator()(const Point & point) const override
  {
    Function function(limitState_);
    function.setParameter(parameter_);
    const Point values(function(point));
    Scalar minimum = values[0];
    for (UnsignedInteger i = 1; i < values.getDimension(); ++i)
      minimum = std::min(minimum, values[i]);
    return Point(1, safeSign_ * minimum);
  }

  UnsignedInteger getInputDimension() const override
  {
    return limitState_.getInputDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return 1;
  }

  Description getInputDescription() const override
  {
    return limitState_.getInputDescription();
  }

  Description getOutputDescription() const override
  {
    return Description(1, "shifted");
  }

private:
  Function limitState_;
  Point parameter_;
  Scalar safeSign_ = 1.0;
};

/* Gradient of the exact shifted constraint */
class ExactShiftedGradient
  : public GradientImplementation
{
public:
  ExactShiftedGradient(const Function & limitState,
                       const Point & parameter,
                       const Scalar safeSign)
    : GradientImplementation()
    , limitState_(limitState)
    , parameter_(parameter)
    , safeSign_(safeSign)
  {
    if (parameter_.getDimension() != limitState_.getParameterDimension())
      throw InvalidDimensionException(HERE) << "The physical shift parameter dimension ("
                                            << parameter_.getDimension()
                                            << ") must match the limit-state parameter dimension ("
                                            << limitState_.getParameterDimension() << ")";
  }

  ExactShiftedGradient * clone() const override
  {
    return new ExactShiftedGradient(*this);
  }

  Matrix gradient(const Point & point) const override
  {
    Function function(limitState_);
    function.setParameter(parameter_);
    const Point values(function(point));
    const Matrix fullGradient(function.gradient(point));
    UnsignedInteger activeIndex = 0;
    Scalar activeValue = values[0];
    for (UnsignedInteger i = 1; i < values.getDimension(); ++i)
      if (values[i] < activeValue)
      {
        activeValue = values[i];
        activeIndex = i;
      }
    Matrix result(1, limitState_.getInputDimension());
    for (UnsignedInteger j = 0; j < limitState_.getInputDimension(); ++j)
      result(0, j) = safeSign_ * fullGradient(activeIndex, j);
    return result;
  }

  UnsignedInteger getInputDimension() const override
  {
    return limitState_.getInputDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return 1;
  }

private:
  Function limitState_;
  Point parameter_;
  Scalar safeSign_ = 1.0;
};

/* Evaluation of the componentwise shifted constraints
   sigma g_j(x; theta_j*), one parameter vector per component. */
class ComponentShiftedEvaluation
  : public EvaluationImplementation
{
public:
  ComponentShiftedEvaluation(const Function & limitState,
                             const Collection<Point> & parameters,
                             const Scalar safeSign)
    : EvaluationImplementation()
    , limitState_(limitState)
    , parameters_(parameters)
    , safeSign_(safeSign)
  {
    if (parameters_.getSize() != limitState_.getOutputDimension())
      throw InvalidDimensionException(HERE) << "The Bonferroni treatment needs one physical design point per limit-state component, got "
                                            << parameters_.getSize() << " points for "
                                            << limitState_.getOutputDimension() << " components";
    for (UnsignedInteger i = 0; i < parameters_.getSize(); ++i)
      if (parameters_[i].getDimension() != limitState_.getParameterDimension())
        throw InvalidDimensionException(HERE) << "The physical shift parameter dimension ("
                                              << parameters_[i].getDimension()
                                              << ") must match the limit-state parameter dimension ("
                                              << limitState_.getParameterDimension() << ")";
  }

  ComponentShiftedEvaluation * clone() const override
  {
    return new ComponentShiftedEvaluation(*this);
  }

  Point operator()(const Point & point) const override
  {
    const UnsignedInteger outputDimension = limitState_.getOutputDimension();
    Point result(outputDimension);
    for (UnsignedInteger i = 0; i < outputDimension; ++i)
    {
      Function function(limitState_);
      function.setParameter(parameters_[i]);
      result[i] = safeSign_ * function(point)[i];
    }
    return result;
  }

  UnsignedInteger getInputDimension() const override
  {
    return limitState_.getInputDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return limitState_.getOutputDimension();
  }

  Description getInputDescription() const override
  {
    return limitState_.getInputDescription();
  }

  Description getOutputDescription() const override
  {
    return limitState_.getOutputDescription();
  }

private:
  Function limitState_;
  Collection<Point> parameters_;
  Scalar safeSign_ = 1.0;
};

/* Gradient of the componentwise shifted constraints */
class ComponentShiftedGradient
  : public GradientImplementation
{
public:
  ComponentShiftedGradient(const Function & limitState,
                           const Collection<Point> & parameters,
                           const Scalar safeSign)
    : GradientImplementation()
    , limitState_(limitState)
    , parameters_(parameters)
    , safeSign_(safeSign)
  {
    if (parameters_.getSize() != limitState_.getOutputDimension())
      throw InvalidDimensionException(HERE) << "The Bonferroni treatment needs one physical design point per limit-state component, got "
                                            << parameters_.getSize() << " points for "
                                            << limitState_.getOutputDimension() << " components";
    for (UnsignedInteger i = 0; i < parameters_.getSize(); ++i)
      if (parameters_[i].getDimension() != limitState_.getParameterDimension())
        throw InvalidDimensionException(HERE) << "The physical shift parameter dimension ("
                                              << parameters_[i].getDimension()
                                              << ") must match the limit-state parameter dimension ("
                                              << limitState_.getParameterDimension() << ")";
  }

  ComponentShiftedGradient * clone() const override
  {
    return new ComponentShiftedGradient(*this);
  }

  Matrix gradient(const Point & point) const override
  {
    const UnsignedInteger outputDimension = limitState_.getOutputDimension();
    const UnsignedInteger inputDimension = limitState_.getInputDimension();
    Matrix result(outputDimension, inputDimension);
    for (UnsignedInteger i = 0; i < outputDimension; ++i)
    {
      Function function(limitState_);
      function.setParameter(parameters_[i]);
      const Matrix fullGradient(function.gradient(point));
      for (UnsignedInteger j = 0; j < inputDimension; ++j)
        result(i, j) = safeSign_ * fullGradient(i, j);
    }
    return result;
  }

  UnsignedInteger getInputDimension() const override
  {
    return limitState_.getInputDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return limitState_.getOutputDimension();
  }

private:
  Function limitState_;
  Collection<Point> parameters_;
  Scalar safeSign_ = 1.0;
};

/* Solve all the reliability components of a joint chance constraint */
Collection<ReliabilityReport> AnalyzeReliability(const RBDOProblem & problem,
                                                 const SORARobustOptimizationAlgorithm::JointChanceTreatment treatment,
                                                 const OptimizationAlgorithm & innerSolver,
                                                 const Point & designPoint,
                                                 const Bool usePMA,
                                                 Collection<Point> & warmStarts)
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
  if ((treatment == SORARobustOptimizationAlgorithm::BONFERRONI) && (outputDimension > 1))
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

  if (warmStarts.getSize() != outputIndices.getSize())
    warmStarts = Collection<Point>(outputIndices.getSize());
  Collection<ReliabilityReport> reports;
  for (UnsignedInteger i = 0; i < outputIndices.getSize(); ++i)
  {
    const Function function(BuildScalarizedFunction(vectorFunction, safeSign, outputIndices[i]));
    ReliabilityReport report;
    report.component = outputIndices[i];
    if (usePMA)
      report.inner = SolvePMA(function, betaTargets[i], innerSolver, warmStarts[i]);
    else
      report.inner = SolveRIA(function, betaTargets[i], innerSolver, warmStarts[i]);
    warmStarts[i] = report.inner.standardDesignPoint;
    report.physicalDesignPoint = inverseTransformation(report.inner.standardDesignPoint);
    if (!IsFinitePoint(report.physicalDesignPoint))
      throw InternalException(HERE) << "The inverse isoprobabilistic transformation returned a non-finite physical design point";
    reports.add(report);
  }
  return reports;
}

/* Index of the worst reliability component */
UnsignedInteger WorstComponent(const Collection<ReliabilityReport> & reports)
{
  UnsignedInteger worst = 0;
  for (UnsignedInteger i = 1; i < reports.getSize(); ++i)
    if (reports[i].inner.constraint < reports[worst].inner.constraint)
      worst = i;
  return worst;
}

/* Physical shifts of all reliability components */
Collection<Point> ComputeShifts(const Point & designPoint,
                                const Collection<ReliabilityReport> & reports)
{
  Collection<Point> shifts;
  for (UnsignedInteger i = 0; i < reports.getSize(); ++i)
    shifts.add(designPoint - reports[i].physicalDesignPoint);
  return shifts;
}

/* Largest change between two sets of shifts */
Scalar MaxShiftChange(const Collection<Point> & previous,
                      const Collection<Point> & current)
{
  if ((previous.getSize() != current.getSize()) || (current.getSize() == 0))
    return std::numeric_limits<Scalar>::infinity();
  Scalar change = 0.0;
  for (UnsignedInteger i = 0; i < current.getSize(); ++i)
    change = std::max(change, (current[i] - previous[i]).norm());
  return change;
}

/* Largest norm of a set of shifts */
Scalar MaxShiftNorm(const Collection<Point> & shifts)
{
  Scalar norm = 0.0;
  for (UnsignedInteger i = 0; i < shifts.getSize(); ++i)
    norm = std::max(norm, shifts[i].norm());
  return norm;
}

/* Build the deterministic shifted chance constraint */
Function BuildShiftedConstraint(const RBDOProblem & problem,
                                const Collection<ReliabilityReport> & reports,
                                const SORARobustOptimizationAlgorithm::JointChanceTreatment treatment)
{
  const Function limitState(problem.getLimitStateFunction());
  const Scalar safeSign = GetSafeSign(problem.getComparisonOperator());
  const UnsignedInteger outputDimension = limitState.getOutputDimension();
  if ((treatment == SORARobustOptimizationAlgorithm::BONFERRONI) && (outputDimension > 1))
  {
    Collection<Point> parameters;
    for (UnsignedInteger i = 0; i < reports.getSize(); ++i)
      parameters.add(reports[i].physicalDesignPoint);
    return Function(Evaluation(new ComponentShiftedEvaluation(limitState, parameters, safeSign)),
                    Gradient(new ComponentShiftedGradient(limitState, parameters, safeSign)),
                    Hessian(new NoHessian()));
  }
  const Point parameter(reports[0].physicalDesignPoint);
  return Function(Evaluation(new ExactShiftedEvaluation(limitState, parameter, safeSign)),
                  Gradient(new ExactShiftedGradient(limitState, parameter, safeSign)),
                  Hessian(new NoHessian()));
}

const char * TreatmentName(const SORARobustOptimizationAlgorithm::JointChanceTreatment treatment)
{
  return treatment == SORARobustOptimizationAlgorithm::BONFERRONI ? "BONFERRONI" : "EXACT";
}

} /* anonymous namespace */

/* Default constructor */
SORARobustOptimizationAlgorithm::SORARobustOptimizationAlgorithm()
  : OptimizationAlgorithmImplementation()
  , solver_(Cobyla())
  , innerSolver_(RBDOAlgorithmTools::BuildDefaultInnerSolver("SORARobustOptimizationAlgorithm"))
{
  // Nothing to do
}

/* Parameter constructor */
SORARobustOptimizationAlgorithm::SORARobustOptimizationAlgorithm(const RBDOProblem & problem,
                                                                 const OptimizationAlgorithm & solver)
  : OptimizationAlgorithmImplementation()
  , solver_(solver)
  , innerSolver_(RBDOAlgorithmTools::BuildDefaultInnerSolver("SORARobustOptimizationAlgorithm"))
{
  setRBDOProblem(problem);
}

/* Parameter constructor */
SORARobustOptimizationAlgorithm::SORARobustOptimizationAlgorithm(const RBDOProblem & problem,
                                                                 const OptimizationAlgorithm & solver,
                                                                 const OptimizationAlgorithm & innerSolver)
  : OptimizationAlgorithmImplementation()
  , solver_(solver)
  , innerSolver_(innerSolver)
{
  setRBDOProblem(problem);
}

/* Virtual constructor method */
SORARobustOptimizationAlgorithm * SORARobustOptimizationAlgorithm::clone() const
{
  return new SORARobustOptimizationAlgorithm(*this);
}

/* Performs the actual computation */
void SORARobustOptimizationAlgorithm::run()
{
  rbdoProblem_.check();
  const ComparisonOperator op(rbdoProblem_.getComparisonOperator());
  const Scalar safeProbability = GetSafeProbability(op, rbdoProblem_.getTargetProbability());
  if (!(safeProbability > 0.5) || !(safeProbability < 1.0))
    throw InvalidArgumentException(HERE) << "SORARobustOptimizationAlgorithm requires a safe-event target probability in (0.5, 1), got "
                                         << safeProbability << " for the joint chance measure with target "
                                         << rbdoProblem_.getTargetProbability() << " and operator " << op
                                         << ". Use DoubleLoopRBDOAlgorithm or SampleAverageApproximationAlgorithm for other target probabilities.";
  (void) GetSafeSign(op);

  const Function limitState(rbdoProblem_.getLimitStateFunction());
  const UnsignedInteger designDimension = rbdoProblem_.getObjective().getInputDimension();
  const UnsignedInteger parameterDimension = limitState.getParameterDimension();
  const UnsignedInteger randomDimension = rbdoProblem_.getDistribution().getDimension();
  if ((designDimension != parameterDimension) || (designDimension != randomDimension))
    throw InvalidDimensionException(HERE) << "SORARobustOptimizationAlgorithm uses the physical shift d = x - theta*, which requires the design dimension ("
                                          << designDimension << ") to match the limit-state parameter dimension ("
                                          << parameterDimension << ") and the distribution dimension ("
                                          << randomDimension << "). Use DoubleLoopRBDOAlgorithm for a problem where the design and random variables have different dimensions.";

  const UnsignedInteger outputDimension = limitState.getOutputDimension();
  Scalar targetAlpha = safeProbability;
  if ((jointChanceTreatment_ == BONFERRONI) && (outputDimension > 1))
    targetAlpha = 1.0 - (1.0 - safeProbability) / outputDimension;
  if (!(targetAlpha > 0.5) || !(targetAlpha < 1.0))
    throw InvalidArgumentException(HERE) << "The safe-event probability associated with the selected joint chance treatment is "
                                         << targetAlpha << ", which is outside (0.5, 1). Use the exact treatment or a sample-based algorithm.";
  (void) DistFunc::qNormal(targetAlpha);

  Point designPoint(buildStartingPoint());
  Collection<Point> warmStarts;

  // Initial reliability analysis at the starting design.
  Collection<ReliabilityReport> reports(AnalyzeReliability(rbdoProblem_,
                                                           jointChanceTreatment_,
                                                           innerSolver_,
                                                           designPoint,
                                                           true,
                                                           warmStarts));
  Collection<Point> shifts(ComputeShifts(designPoint, reports));

  const Scalar maximumAbsoluteError = getMaximumAbsoluteError();
  const Scalar maximumRelativeError = getMaximumRelativeError();
  const UnsignedInteger maximumIterationNumber = getMaximumIterationNumber();
  const Bool hasProgressCallback = progressCallback_.first != nullptr;

  UnsignedInteger iteration = 0;
  Bool converged = false;
  Bool interrupted = false;
  Scalar designAbsoluteError = std::numeric_limits<Scalar>::infinity();
  Scalar designRelativeError = std::numeric_limits<Scalar>::infinity();
  Scalar shiftAbsoluteError = std::numeric_limits<Scalar>::infinity();
  Scalar shiftRelativeError = std::numeric_limits<Scalar>::infinity();

  for (; iteration < maximumIterationNumber; ++iteration)
  {
    if (stopCallback_.first && stopCallback_.first(stopCallback_.second))
    {
      interrupted = true;
      break;
    }

    // Deterministic optimization with the shifted chance constraint.
    const Function chanceConstraint(BuildShiftedConstraint(rbdoProblem_, reports, jointChanceTreatment_));
    OptimizationProblem deterministicProblem;
    deterministicProblem.setObjective(rbdoProblem_.getObjective());
    if (rbdoProblem_.getBounds().getDimension() != 0)
      deterministicProblem.setBounds(rbdoProblem_.getBounds());
    if (rbdoProblem_.getVariablesType().getSize() != 0)
      deterministicProblem.setVariablesType(rbdoProblem_.getVariablesType());
    AggregatedFunction::FunctionCollection inequalities;
    inequalities.add(chanceConstraint);
    if (rbdoProblem_.hasInequalityConstraint())
      inequalities.add(rbdoProblem_.getInequalityConstraint());
    deterministicProblem.setInequalityConstraint(AggregatedFunction(inequalities));
    if (rbdoProblem_.hasEqualityConstraint())
      deterministicProblem.setEqualityConstraint(rbdoProblem_.getEqualityConstraint());
    deterministicProblem.setMinimization(rbdoProblem_.isMinimization(0), 0);

    OptimizationAlgorithm solver(solver_);
    solver.setProblem(deterministicProblem);
    solver.setStartingPoint(designPoint);
    solver.run();
    const OptimizationResult outerResult(solver.getResult());
    const Point previousDesignPoint(designPoint);
    designPoint = outerResult.getOptimalPoint();
    if (designPoint.getDimension() != designDimension)
      throw InternalException(HERE) << "The deterministic SORA subproblem returned a design point of dimension "
                                   << designPoint.getDimension() << " instead of " << designDimension;
    if (!IsFinitePoint(designPoint))
      throw InternalException(HERE) << "The deterministic SORA subproblem returned a non-finite design point: "
                                   << designPoint;

    // Reliability analysis at the new design and shift update.
    const Collection<ReliabilityReport> nextReports(AnalyzeReliability(rbdoProblem_,
                                                                      jointChanceTreatment_,
                                                                      innerSolver_,
                                                                      designPoint,
                                                                      true,
                                                                      warmStarts));
    const Collection<Point> nextShifts(ComputeShifts(designPoint, nextReports));
    designAbsoluteError = (designPoint - previousDesignPoint).norm();
    designRelativeError = designAbsoluteError / std::max(designPoint.norm(), 1.0);
    shiftAbsoluteError = MaxShiftChange(shifts, nextShifts);
    shiftRelativeError = shiftAbsoluteError / std::max(MaxShiftNorm(nextShifts), 1.0);
    reports = nextReports;
    shifts = nextShifts;

    if (hasProgressCallback)
      progressCallback_.first(100.0 * (iteration + 1) / maximumIterationNumber, progressCallback_.second);

    const Bool designConverged = (designAbsoluteError <= maximumAbsoluteError)
                                 || (designRelativeError <= maximumRelativeError);
    const Bool shiftConverged = (shiftAbsoluteError <= maximumAbsoluteError)
                                || (shiftRelativeError <= maximumRelativeError);
    if (designConverged && shiftConverged)
    {
      converged = true;
      ++iteration;
      break;
    }
  }

  // Essential PMA report at the final design.
  const UnsignedInteger worst = WorstComponent(reports);
  performanceMeasure_ = reports[worst].inner.performance;
  standardSpaceDesignPoint_ = reports[worst].inner.standardDesignPoint;
  physicalSpaceDesignPoint_ = reports[worst].physicalDesignPoint;
  shift_ = designPoint - physicalSpaceDesignPoint_;

  // Diagnostic RIA report, used for the reliability index and probability.
  reliabilityIndex_ = NaN;
  try
  {
    Collection<Point> diagnosticWarmStarts;
    const Collection<ReliabilityReport> diagnosticReports(AnalyzeReliability(rbdoProblem_,
                                                                            jointChanceTreatment_,
                                                                            innerSolver_,
                                                                            designPoint,
                                                                            false,
                                                                            diagnosticWarmStarts));
    reliabilityIndex_ = diagnosticReports[WorstComponent(diagnosticReports)].inner.beta;
  }
  catch (const Exception &)
  {
    // The diagnostic analysis is not essential to the result.
  }
  approximateProbability_ = std::isfinite(reliabilityIndex_) ? DistFunc::pNormal(reliabilityIndex_) : NaN;

  OptimizationResult result(rbdoProblem_);
  result.setOptimalPoint(designPoint);
  result.setOptimalValue(rbdoProblem_.getObjective()(designPoint));
  result.setIterationNumber(iteration);
  if (converged)
  {
    result.setStatus(OptimizationResult::SUCCESS);
    OSS oss;
    oss << "SORA converged after " << iteration << " iteration(s) with "
        << TreatmentName(jointChanceTreatment_) << " joint chance treatment";
    result.setStatusMessage(oss);
  }
  else if (interrupted)
  {
    result.setStatus(OptimizationResult::INTERRUPTION);
    OSS oss;
    oss << "SORA was interrupted after " << iteration << " iteration(s) with "
        << TreatmentName(jointChanceTreatment_) << " joint chance treatment";
    result.setStatusMessage(oss);
  }
  else
  {
    result.setStatus(OptimizationResult::FAILURE);
    OSS oss;
    oss << "SORA did not converge after " << iteration << " iteration(s) with "
        << TreatmentName(jointChanceTreatment_) << " joint chance treatment"
        << ": design absolute error=" << designAbsoluteError
        << ", design relative error=" << designRelativeError
        << ", shift absolute error=" << shiftAbsoluteError
        << ", shift relative error=" << shiftRelativeError;
    result.setStatusMessage(oss);
  }
  result_ = result;
  hasRun_ = true;
}

/* Problem accessor */
void SORARobustOptimizationAlgorithm::setProblem(const OptimizationProblem & problem)
{
  const RBDOProblem * p_problem = dynamic_cast<const RBDOProblem *>(problem.getImplementation().get());
  if (p_problem == nullptr)
    throw InvalidArgumentException(HERE) << "SORARobustOptimizationAlgorithm expects an RBDOProblem, got "
                                         << problem.getImplementation()->getClassName();
  OptimizationAlgorithmImplementation::setProblem(problem);
  rbdoProblem_ = *p_problem;
  hasRun_ = false;
}

void SORARobustOptimizationAlgorithm::setRBDOProblem(const RBDOProblem & problem)
{
  setProblem(problem);
}

RBDOProblem SORARobustOptimizationAlgorithm::getRBDOProblem() const
{
  return rbdoProblem_;
}

/* Outer deterministic solver accessors */
void SORARobustOptimizationAlgorithm::setOptimizationAlgorithm(const OptimizationAlgorithm & solver)
{
  solver_ = solver;
  hasRun_ = false;
}

OptimizationAlgorithm SORARobustOptimizationAlgorithm::getOptimizationAlgorithm() const
{
  return solver_;
}

/* Inner reliability solver accessors */
void SORARobustOptimizationAlgorithm::setInnerOptimizationAlgorithm(const OptimizationAlgorithm & solver)
{
  innerSolver_ = solver;
  hasRun_ = false;
}

OptimizationAlgorithm SORARobustOptimizationAlgorithm::getInnerOptimizationAlgorithm() const
{
  return innerSolver_;
}

/* Joint chance treatment accessors */
void SORARobustOptimizationAlgorithm::setJointChanceTreatment(const JointChanceTreatment treatment)
{
  if ((treatment != EXACT) && (treatment != BONFERRONI))
    throw InvalidArgumentException(HERE) << "The joint chance treatment must be EXACT or BONFERRONI";
  jointChanceTreatment_ = treatment;
  hasRun_ = false;
}

SORARobustOptimizationAlgorithm::JointChanceTreatment SORARobustOptimizationAlgorithm::getJointChanceTreatment() const
{
  return jointChanceTreatment_;
}

/* Target reliability index defined by the joint chance constraint */
Scalar SORARobustOptimizationAlgorithm::getTargetReliabilityIndex() const
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

/* Diagnostic reliability index at the solution */
Scalar SORARobustOptimizationAlgorithm::getReliabilityIndex() const
{
  if (!hasRun_)
    throw NotDefinedException(HERE) << "run() must be called before getReliabilityIndex()";
  return reliabilityIndex_;
}

/* Performance measure at the solution */
Scalar SORARobustOptimizationAlgorithm::getPerformanceMeasure() const
{
  if (!hasRun_)
    throw NotDefinedException(HERE) << "run() must be called before getPerformanceMeasure()";
  return performanceMeasure_;
}

/* Physical shift of the worst component at the solution */
Point SORARobustOptimizationAlgorithm::getShift() const
{
  if (!hasRun_)
    throw NotDefinedException(HERE) << "run() must be called before getShift()";
  return shift_;
}

/* Standard-space design point of the worst component at the solution */
Point SORARobustOptimizationAlgorithm::getStandardSpaceDesignPoint() const
{
  if (!hasRun_)
    throw NotDefinedException(HERE) << "run() must be called before getStandardSpaceDesignPoint()";
  return standardSpaceDesignPoint_;
}

/* Physical-space design point of the worst component at the solution */
Point SORARobustOptimizationAlgorithm::getPhysicalSpaceDesignPoint() const
{
  if (!hasRun_)
    throw NotDefinedException(HERE) << "run() must be called before getPhysicalSpaceDesignPoint()";
  return physicalSpaceDesignPoint_;
}

/* FORM approximation of the safe probability at the solution */
Scalar SORARobustOptimizationAlgorithm::getApproximateProbability() const
{
  if (!hasRun_)
    throw NotDefinedException(HERE) << "run() must be called before getApproximateProbability()";
  return approximateProbability_;
}

/* Check whether this problem can be solved by this solver */
void SORARobustOptimizationAlgorithm::checkProblem(const OptimizationProblem & problem) const
{
  const RBDOProblem * p_problem = dynamic_cast<const RBDOProblem *>(problem.getImplementation().get());
  if (p_problem == nullptr)
    throw InvalidArgumentException(HERE) << "SORARobustOptimizationAlgorithm expects an RBDOProblem, got "
                                         << problem.getImplementation()->getClassName();
  p_problem->check();
  const ComparisonOperator op(p_problem->getComparisonOperator());
  const Scalar safeProbability = GetSafeProbability(op, p_problem->getTargetProbability());
  if (!(safeProbability > 0.5) || !(safeProbability < 1.0))
    throw InvalidArgumentException(HERE) << "SORARobustOptimizationAlgorithm requires a safe-event target probability in (0.5, 1), got "
                                         << safeProbability << " for the joint chance measure with target "
                                         << p_problem->getTargetProbability() << " and operator " << op
                                         << ". Use DoubleLoopRBDOAlgorithm or SampleAverageApproximationAlgorithm for other target probabilities.";
  (void) GetSafeSign(op);
  const Function limitState(p_problem->getLimitStateFunction());
  const UnsignedInteger designDimension = p_problem->getObjective().getInputDimension();
  const UnsignedInteger parameterDimension = limitState.getParameterDimension();
  const UnsignedInteger randomDimension = p_problem->getDistribution().getDimension();
  if ((designDimension != parameterDimension) || (designDimension != randomDimension))
    throw InvalidDimensionException(HERE) << "SORARobustOptimizationAlgorithm uses the physical shift d = x - theta*, which requires the design dimension ("
                                          << designDimension << ") to match the limit-state parameter dimension ("
                                          << parameterDimension << ") and the distribution dimension ("
                                          << randomDimension << "). Use DoubleLoopRBDOAlgorithm for a problem where the design and random variables have different dimensions.";
}

/* Build a starting point for the outer optimization */
Point SORARobustOptimizationAlgorithm::buildStartingPoint() const
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

Bool SORARobustOptimizationAlgorithm::hasRun() const
{
  return hasRun_;
}

/* String converter */
String SORARobustOptimizationAlgorithm::__repr__() const
{
  OSS oss;
  oss << "class=" << SORARobustOptimizationAlgorithm::GetClassName()
      << " jointChanceTreatment=" << TreatmentName(jointChanceTreatment_)
      << " problem=" << rbdoProblem_
      << " solver=" << solver_
      << " innerSolver=" << innerSolver_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void SORARobustOptimizationAlgorithm::save(Advocate & adv) const
{
  OptimizationAlgorithmImplementation::save(adv);
  adv.saveAttribute("rbdoProblem_", rbdoProblem_);
  adv.saveAttribute("jointChanceTreatment_", static_cast<UnsignedInteger>(jointChanceTreatment_));
  adv.saveAttribute("solver_", solver_);
  adv.saveAttribute("innerSolver_", innerSolver_);
  adv.saveAttribute("reliabilityIndex_", reliabilityIndex_);
  adv.saveAttribute("performanceMeasure_", performanceMeasure_);
  adv.saveAttribute("approximateProbability_", approximateProbability_);
  adv.saveAttribute("shift_", shift_);
  adv.saveAttribute("standardSpaceDesignPoint_", standardSpaceDesignPoint_);
  adv.saveAttribute("physicalSpaceDesignPoint_", physicalSpaceDesignPoint_);
  adv.saveAttribute("hasRun_", hasRun_);
}

/* Method load() reloads the object from the StorageManager */
void SORARobustOptimizationAlgorithm::load(Advocate & adv)
{
  OptimizationAlgorithmImplementation::load(adv);
  adv.loadAttribute("rbdoProblem_", rbdoProblem_);
  UnsignedInteger treatment = 0;
  adv.loadAttribute("jointChanceTreatment_", treatment);
  jointChanceTreatment_ = static_cast<JointChanceTreatment>(treatment);
  adv.loadAttribute("solver_", solver_);
  adv.loadAttribute("innerSolver_", innerSolver_);
  adv.loadAttribute("reliabilityIndex_", reliabilityIndex_);
  adv.loadAttribute("performanceMeasure_", performanceMeasure_);
  adv.loadAttribute("approximateProbability_", approximateProbability_);
  adv.loadAttribute("shift_", shift_);
  adv.loadAttribute("standardSpaceDesignPoint_", standardSpaceDesignPoint_);
  adv.loadAttribute("physicalSpaceDesignPoint_", physicalSpaceDesignPoint_);
  adv.loadAttribute("hasRun_", hasRun_);
}

} /* namespace OTROBOPT */
