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
#include "otrobopt/RBDOAlgorithmTools.hxx"

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

using namespace RBDOAlgorithmTools;

CLASSNAMEINIT(DoubleLoopRBDOAlgorithm)

static Factory<DoubleLoopRBDOAlgorithm> Factory_DoubleLoopRBDOAlgorithm;

namespace
{

const Scalar NaN = std::numeric_limits<Scalar>::quiet_NaN();

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
  , innerSolver_(RBDOAlgorithmTools::BuildDefaultInnerSolver("DoubleLoopRBDOAlgorithm"))
{
  // Nothing to do
}

/* Parameter constructor */
DoubleLoopRBDOAlgorithm::DoubleLoopRBDOAlgorithm(const RBDOProblem & problem,
                                                 const OptimizationAlgorithm & solver)
  : OptimizationAlgorithmImplementation()
  , solver_(solver)
  , innerSolver_(RBDOAlgorithmTools::BuildDefaultInnerSolver("DoubleLoopRBDOAlgorithm"))
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
