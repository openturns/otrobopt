//                                               -*- C++ -*-
/**
 *  @brief Sample-average approximation for reliability-based design optimization
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
#include "otrobopt/SampleAverageApproximationAlgorithm.hxx"

#include <openturns/AggregatedFunction.hxx>
#include <openturns/Cobyla.hxx>
#include <openturns/DistFunc.hxx>
#include <openturns/Distribution.hxx>
#include <openturns/EvaluationImplementation.hxx>
#include <openturns/Exception.hxx>
#include <openturns/Interval.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/ResourceMap.hxx>

#include <cmath>
#include <limits>
#include <memory>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(SampleAverageApproximationAlgorithm)

static Factory<SampleAverageApproximationAlgorithm> Factory_SampleAverageApproximationAlgorithm;

namespace
{

/* Whether a point has only finite coordinates */
Bool HasFiniteCoordinates(const Point & point)
{
  for (UnsignedInteger i = 0; i < point.getDimension(); ++i)
    if (!std::isfinite(point[i]))
      return false;
  return true;
}

/* Safe event: all components are nonnegative for a Greater operator, and at
   least one component is negative for a Less operator.  The latter is the
   complement of the event integrated by JointChanceMeasure. */
Bool IsSafe(const Point & value,
            const Bool greater)
{
  if (greater)
  {
    for (UnsignedInteger j = 0; j < value.getDimension(); ++j)
      if (value[j] < 0.0)
        return false;
    return true;
  }
  for (UnsignedInteger j = 0; j < value.getDimension(); ++j)
    if (value[j] < 0.0)
      return true;
  return false;
}

/* Empirical safe probability of a given design on a fixed sample */
Scalar ComputeSafeProbability(const Function & limitState,
                              const Sample & sample,
                              const Point & designPoint,
                              const Bool greater)
{
  const UnsignedInteger size = sample.getSize();
  if (size == 0)
    throw InvalidArgumentException(HERE) << "The sample must not be empty";
  UnsignedInteger count = 0;
  for (UnsignedInteger i = 0; i < size; ++i)
  {
    Function function(limitState);
    function.setParameter(sample[i]);
    const Point value(function(designPoint));
    if (!HasFiniteCoordinates(value))
      throw InternalException(HERE) << "The limit-state function returned a non-finite value for sample index " << i;
    if (IsSafe(value, greater))
      ++count;
  }
  return static_cast<Scalar>(count) / static_cast<Scalar>(size);
}

/* Wilson score interval for a binomial proportion */
Interval WilsonInterval(const Scalar probability,
                        const UnsignedInteger size,
                        const Scalar level)
{
  if (size == 0)
    throw InvalidArgumentException(HERE) << "The validation sample must not be empty";
  const Scalar z = DistFunc::qNormal(0.5 * (1.0 + level));
  const Scalar z2 = z * z;
  const Scalar p = probability;
  const Scalar denominator = 1.0 + z2 / size;
  const Scalar center = (p + 0.5 * z2 / size) / denominator;
  const Scalar halfWidth = z * std::sqrt(p * (1.0 - p) / size + 0.25 * z2 / (static_cast<Scalar>(size) * size)) / denominator;
  return Interval(std::max(0.0, center - halfWidth),
                  std::min(1.0, center + halfWidth));
}

/* Shared state of the empirical chance constraint */
class EmpiricalChanceState
{
public:
  EmpiricalChanceState(const RBDOProblem & problem,
                       const Sample & sample)
    : problem_(problem)
    , sample_(sample)
    , limitState_(problem.getLimitStateFunction())
    , greater_(problem.getComparisonOperator()(2.0, 1.0) && !problem.getComparisonOperator()(1.0, 2.0))
    , safeTarget_(greater_ ? problem.getTargetProbability() : 1.0 - problem.getTargetProbability())
  {
    // Nothing to do
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
        return lastValue_;
    }
    const Scalar probability = ComputeSafeProbability(limitState_, sample_, designPoint, greater_);
    lastPoint_ = designPoint;
    lastValue_ = probability - safeTarget_;
    hasLastPoint_ = true;
    return lastValue_;
  }

private:
  RBDOProblem problem_;
  Sample sample_;
  Function limitState_;
  Bool greater_ = true;
  Scalar safeTarget_ = 0.0;
  Point lastPoint_;
  Scalar lastValue_ = 0.0;
  Bool hasLastPoint_ = false;
};

/* Clamp a point to the finite bounds of the problem */
Point ClampToBounds(const RBDOProblem & problem,
                    const Point & point)
{
  const Interval bounds(problem.getBounds());
  if (bounds.getDimension() != point.getDimension())
    return point;
  const Point lower(bounds.getLowerBound());
  const Point upper(bounds.getUpperBound());
  Point result(point);
  for (UnsignedInteger i = 0; i < result.getDimension(); ++i)
  {
    if (std::isfinite(lower[i]) && (result[i] < lower[i]))
      result[i] = lower[i];
    if (std::isfinite(upper[i]) && (result[i] > upper[i]))
      result[i] = upper[i];
  }
  return result;
}

/* Direction that increases the scalarized limit state at a reference
   realization.  It is used only to find a feasible starting point. */
Point FindSafeDirection(const Function & limitState,
                        const Point & reference,
                        const Point & designPoint,
                        const Scalar safeSign)
{
  const UnsignedInteger dimension = designPoint.getDimension();
  Function function(limitState);
  function.setParameter(reference);
  const Point value(function(designPoint));
  UnsignedInteger activeIndex = 0;
  for (UnsignedInteger j = 1; j < value.getDimension(); ++j)
    if (value[j] < value[activeIndex])
      activeIndex = j;

  Point direction(dimension);
  try
  {
    const Matrix gradient(function.gradient(designPoint));
    if ((gradient.getNbRows() == value.getDimension()) && (gradient.getNbColumns() == dimension))
    {
      for (UnsignedInteger j = 0; j < dimension; ++j)
        direction[j] = safeSign * gradient(activeIndex, j);
      if (direction.norm() > 0.0)
        return direction;
    }
  }
  catch (const Exception &)
  {
    // Fall back to finite differences
  }

  const Scalar step = 1e-4;
  Point perturbed(designPoint);
  for (UnsignedInteger j = 0; j < dimension; ++j)
  {
    const Scalar h = step * std::max(1.0, std::abs(designPoint[j]));
    perturbed[j] = designPoint[j] + h;
    const Point shifted(function(perturbed));
    Scalar shiftedMinimum = shifted[0];
    for (UnsignedInteger k = 1; k < shifted.getDimension(); ++k)
      shiftedMinimum = std::min(shiftedMinimum, shifted[k]);
    perturbed[j] = designPoint[j];
    direction[j] = safeSign * (shiftedMinimum - value[activeIndex]) / h;
  }
  return direction;
}

/* Look for a design that satisfies the empirical chance constraint.  The
   outer solver may otherwise fail to find a feasible point because the
   empirical probability is a step function. */
Point FindFeasiblePoint(const RBDOProblem & problem,
                        const Sample & sample,
                        const Bool greater,
                        const Point & startingPoint,
                        const std::shared_ptr<EmpiricalChanceState> & state)
{
  if (state->evaluate(startingPoint) >= 0.0)
    return startingPoint;

  const UnsignedInteger dimension = startingPoint.getDimension();
  const Point reference(sample.computeMean());
  const Scalar safeSign = greater ? 1.0 : -1.0;
  const Point direction(FindSafeDirection(problem.getLimitStateFunction(),
                                          reference,
                                          startingPoint,
                                          safeSign));

  Scalar scale = 1.0;
  const Interval bounds(problem.getBounds());
  if (bounds.getDimension() == dimension)
  {
    const Point lower(bounds.getLowerBound());
    const Point upper(bounds.getUpperBound());
    for (UnsignedInteger i = 0; i < dimension; ++i)
      if (std::isfinite(lower[i]) && std::isfinite(upper[i]))
        scale = std::max(scale, 0.25 * (upper[i] - lower[i]));
  }
  const Scalar steps[] = {0.05, 0.1, 0.25, 0.5, 1.0, 2.0, 4.0, 8.0, 16.0};

  if (direction.norm() > 0.0)
  {
    const Point unit(direction / direction.norm());
    for (const Scalar sign : {-1.0, 1.0})
      for (const Scalar step : steps)
      {
        const Point candidate(ClampToBounds(problem, startingPoint + (sign * scale * step) * unit));
        if (state->evaluate(candidate) >= 0.0)
          return candidate;
      }
  }

  for (UnsignedInteger i = 0; i < dimension; ++i)
    for (const Scalar sign : {-1.0, 1.0})
      for (const Scalar step : steps)
      {
        Point candidate(startingPoint);
        candidate[i] += sign * scale * step;
        candidate = ClampToBounds(problem, candidate);
        if (state->evaluate(candidate) >= 0.0)
          return candidate;
      }
  return Point();
}

/* Chance constraint function used by the outer solver */
class EmpiricalChanceEvaluation
  : public EvaluationImplementation
{
public:
  explicit EmpiricalChanceEvaluation(const std::shared_ptr<EmpiricalChanceState> & state)
    : EvaluationImplementation()
    , state_(state)
  {
    // Nothing to do
  }

  EmpiricalChanceEvaluation * clone() const override
  {
    return new EmpiricalChanceEvaluation(*this);
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
  std::shared_ptr<EmpiricalChanceState> state_;
};

} /* anonymous namespace */

/* Default constructor */
SampleAverageApproximationAlgorithm::SampleAverageApproximationAlgorithm()
  : OptimizationAlgorithmImplementation()
  , solver_(Cobyla())
{
  sampleSize_ = ResourceMap::GetAsUnsignedInteger("SampleAverageApproximationAlgorithm-DefaultSampleSize");
  validationSampleSize_ = ResourceMap::GetAsUnsignedInteger("SampleAverageApproximationAlgorithm-DefaultValidationSampleSize");
}

/* Parameter constructor */
SampleAverageApproximationAlgorithm::SampleAverageApproximationAlgorithm(const RBDOProblem & problem,
                                                                         const OptimizationAlgorithm & solver)
  : OptimizationAlgorithmImplementation()
  , solver_(solver)
{
  sampleSize_ = ResourceMap::GetAsUnsignedInteger("SampleAverageApproximationAlgorithm-DefaultSampleSize");
  validationSampleSize_ = ResourceMap::GetAsUnsignedInteger("SampleAverageApproximationAlgorithm-DefaultValidationSampleSize");
  setRBDOProblem(problem);
}

/* Parameter constructor with a calibration sample size */
SampleAverageApproximationAlgorithm::SampleAverageApproximationAlgorithm(const RBDOProblem & problem,
                                                                         const OptimizationAlgorithm & solver,
                                                                         const UnsignedInteger sampleSize)
  : OptimizationAlgorithmImplementation()
  , solver_(solver)
{
  sampleSize_ = ResourceMap::GetAsUnsignedInteger("SampleAverageApproximationAlgorithm-DefaultSampleSize");
  validationSampleSize_ = ResourceMap::GetAsUnsignedInteger("SampleAverageApproximationAlgorithm-DefaultValidationSampleSize");
  setSampleSize(sampleSize);
  setRBDOProblem(problem);
}

/* Virtual constructor method */
SampleAverageApproximationAlgorithm * SampleAverageApproximationAlgorithm::clone() const
{
  return new SampleAverageApproximationAlgorithm(*this);
}

/* Performs the actual computation */
void SampleAverageApproximationAlgorithm::run()
{
  rbdoProblem_.check();
  const ComparisonOperator op(rbdoProblem_.getComparisonOperator());
  const Bool greater = op(2.0, 1.0) && !op(1.0, 2.0);
  const Bool less = op(1.0, 2.0) && !op(2.0, 1.0);
  if (greater == less)
    throw InvalidArgumentException(HERE) << "The joint chance constraint operator must be a Greater or a Less comparison, got " << op;

  sample_ = buildCalibrationSample();
  if (sample_.getSize() == 0)
    throw InvalidArgumentException(HERE) << "The calibration sample must not be empty";

  const std::shared_ptr<EmpiricalChanceState> state(new EmpiricalChanceState(rbdoProblem_, sample_));
  Function chanceFunction(Evaluation(new EmpiricalChanceEvaluation(state)));

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
  Point startingPoint(buildStartingPoint());
  const Point feasiblePoint(FindFeasiblePoint(rbdoProblem_, sample_, greater, startingPoint, state));
  if (feasiblePoint.getDimension() == startingPoint.getDimension())
    startingPoint = feasiblePoint;
  solver.setStartingPoint(startingPoint);
  solver.run();
  result_ = solver.getResult();

  const Point optimalPoint(result_.getOptimalPoint());
  if (optimalPoint.getDimension() != rbdoProblem_.getObjective().getInputDimension())
    throw InternalException(HERE) << "The outer optimization did not return a design point of dimension "
                                 << rbdoProblem_.getObjective().getInputDimension();

  safeProbability_ = ComputeSafeProbability(rbdoProblem_.getLimitStateFunction(), sample_, optimalPoint, greater);

  validationSample_ = buildValidationSample();
  if (validationSample_.getSize() == 0)
    throw InvalidArgumentException(HERE) << "The validation sample must not be empty";
  validationSafeProbability_ = ComputeSafeProbability(rbdoProblem_.getLimitStateFunction(),
                                                      validationSample_,
                                                      optimalPoint,
                                                      greater);
  validationConfidenceInterval_ = WilsonInterval(validationSafeProbability_,
                                                 validationSample_.getSize(),
                                                 0.95);
  hasRun_ = true;
}

/* Problem accessor */
void SampleAverageApproximationAlgorithm::setProblem(const OptimizationProblem & problem)
{
  const RBDOProblem * p_problem = dynamic_cast<const RBDOProblem *>(problem.getImplementation().get());
  if (p_problem == nullptr)
    throw InvalidArgumentException(HERE) << "SampleAverageApproximationAlgorithm expects an RBDOProblem, got "
                                         << problem.getImplementation()->getClassName();
  OptimizationAlgorithmImplementation::setProblem(problem);
  rbdoProblem_ = *p_problem;
  hasRun_ = false;
}

void SampleAverageApproximationAlgorithm::setRBDOProblem(const RBDOProblem & problem)
{
  setProblem(problem);
}

RBDOProblem SampleAverageApproximationAlgorithm::getRBDOProblem() const
{
  return rbdoProblem_;
}

/* Outer optimization solver accessors */
void SampleAverageApproximationAlgorithm::setOptimizationAlgorithm(const OptimizationAlgorithm & solver)
{
  solver_ = solver;
  hasRun_ = false;
}

OptimizationAlgorithm SampleAverageApproximationAlgorithm::getOptimizationAlgorithm() const
{
  return solver_;
}

/* Calibration sample size accessors */
void SampleAverageApproximationAlgorithm::setSampleSize(const UnsignedInteger sampleSize)
{
  if (sampleSize == 0)
    throw InvalidArgumentException(HERE) << "The calibration sample size must be positive";
  sampleSize_ = sampleSize;
  hasRun_ = false;
}

UnsignedInteger SampleAverageApproximationAlgorithm::getSampleSize() const
{
  return sampleSize_;
}

/* Calibration sample accessors */
void SampleAverageApproximationAlgorithm::setSample(const Sample & sample)
{
  sample_ = sample;
  sampleProvided_ = sample.getSize() > 0;
  hasRun_ = false;
}

Sample SampleAverageApproximationAlgorithm::getSample() const
{
  return sample_;
}

/* Validation sample size accessors */
void SampleAverageApproximationAlgorithm::setValidationSampleSize(const UnsignedInteger sampleSize)
{
  if (sampleSize == 0)
    throw InvalidArgumentException(HERE) << "The validation sample size must be positive";
  validationSampleSize_ = sampleSize;
  hasRun_ = false;
}

UnsignedInteger SampleAverageApproximationAlgorithm::getValidationSampleSize() const
{
  return validationSampleSize_;
}

/* Validation sample accessors */
void SampleAverageApproximationAlgorithm::setValidationSample(const Sample & sample)
{
  validationSample_ = sample;
  validationSampleProvided_ = sample.getSize() > 0;
  hasRun_ = false;
}

Sample SampleAverageApproximationAlgorithm::getValidationSample() const
{
  return validationSample_;
}

/* Empirical safe probability at the solution on the calibration sample */
Scalar SampleAverageApproximationAlgorithm::getSafeProbability() const
{
  if (!hasRun_)
    throw NotDefinedException(HERE) << "run() must be called before getSafeProbability()";
  return safeProbability_;
}

/* Empirical safe probability at the solution on the validation sample */
Scalar SampleAverageApproximationAlgorithm::getValidationSafeProbability() const
{
  if (!hasRun_)
    throw NotDefinedException(HERE) << "run() must be called before getValidationSafeProbability()";
  return validationSafeProbability_;
}

/* Wilson confidence interval of the validation safe probability */
Interval SampleAverageApproximationAlgorithm::getValidationConfidenceInterval() const
{
  if (!hasRun_)
    throw NotDefinedException(HERE) << "run() must be called before getValidationConfidenceInterval()";
  return validationConfidenceInterval_;
}

/* Check whether this problem can be solved by this solver */
void SampleAverageApproximationAlgorithm::checkProblem(const OptimizationProblem & problem) const
{
  const RBDOProblem * p_problem = dynamic_cast<const RBDOProblem *>(problem.getImplementation().get());
  if (p_problem == nullptr)
    throw InvalidArgumentException(HERE) << "SampleAverageApproximationAlgorithm expects an RBDOProblem, got "
                                         << problem.getImplementation()->getClassName();
  p_problem->check();
  const ComparisonOperator op(p_problem->getComparisonOperator());
  const Bool greater = op(2.0, 1.0) && !op(1.0, 2.0);
  const Bool less = op(1.0, 2.0) && !op(2.0, 1.0);
  if (greater == less)
    throw InvalidArgumentException(HERE) << "The joint chance constraint operator must be a Greater or a Less comparison, got " << op;
  if ((sampleSize_ == 0) && (sample_.getSize() == 0))
    throw InvalidArgumentException(HERE) << "The calibration sample size must be positive";
  if ((validationSampleSize_ == 0) && (validationSample_.getSize() == 0))
    throw InvalidArgumentException(HERE) << "The validation sample size must be positive";
}

/* Build the calibration sample */
Sample SampleAverageApproximationAlgorithm::buildCalibrationSample() const
{
  if (sampleProvided_)
  {
    if (sample_.getDimension() != rbdoProblem_.getDistribution().getDimension())
      throw InvalidDimensionException(HERE) << "The calibration sample dimension ("
                                            << sample_.getDimension()
                                            << ") must match the distribution dimension ("
                                            << rbdoProblem_.getDistribution().getDimension() << ")";
    return sample_;
  }
  return rbdoProblem_.getDistribution().getSample(sampleSize_);
}

/* Build the validation sample */
Sample SampleAverageApproximationAlgorithm::buildValidationSample() const
{
  if (validationSampleProvided_)
  {
    if (validationSample_.getDimension() != rbdoProblem_.getDistribution().getDimension())
      throw InvalidDimensionException(HERE) << "The validation sample dimension ("
                                            << validationSample_.getDimension()
                                            << ") must match the distribution dimension ("
                                            << rbdoProblem_.getDistribution().getDimension() << ")";
    return validationSample_;
  }
  return rbdoProblem_.getDistribution().getSample(validationSampleSize_);
}

/* Build a starting point for the outer optimization */
Point SampleAverageApproximationAlgorithm::buildStartingPoint() const
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
    if ((canonicalStartingPoint.getDimension() == dimension) && HasFiniteCoordinates(canonicalStartingPoint))
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

/* String converter */
String SampleAverageApproximationAlgorithm::__repr__() const
{
  OSS oss;
  oss << "class=" << SampleAverageApproximationAlgorithm::GetClassName()
      << " sampleSize=" << sampleSize_
      << " validationSampleSize=" << validationSampleSize_
      << " problem=" << rbdoProblem_
      << " solver=" << solver_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void SampleAverageApproximationAlgorithm::save(Advocate & adv) const
{
  OptimizationAlgorithmImplementation::save(adv);
  adv.saveAttribute("rbdoProblem_", rbdoProblem_);
  adv.saveAttribute("solver_", solver_);
  adv.saveAttribute("sampleSize_", sampleSize_);
  adv.saveAttribute("validationSampleSize_", validationSampleSize_);
  adv.saveAttribute("sample_", sample_);
  adv.saveAttribute("validationSample_", validationSample_);
  adv.saveAttribute("sampleProvided_", sampleProvided_);
  adv.saveAttribute("validationSampleProvided_", validationSampleProvided_);
  adv.saveAttribute("safeProbability_", safeProbability_);
  adv.saveAttribute("validationSafeProbability_", validationSafeProbability_);
  adv.saveAttribute("validationConfidenceInterval_", validationConfidenceInterval_);
  adv.saveAttribute("hasRun_", hasRun_);
}

/* Method load() reloads the object from the StorageManager */
void SampleAverageApproximationAlgorithm::load(Advocate & adv)
{
  OptimizationAlgorithmImplementation::load(adv);
  adv.loadAttribute("rbdoProblem_", rbdoProblem_);
  adv.loadAttribute("solver_", solver_);
  adv.loadAttribute("sampleSize_", sampleSize_);
  adv.loadAttribute("validationSampleSize_", validationSampleSize_);
  adv.loadAttribute("sample_", sample_);
  adv.loadAttribute("validationSample_", validationSample_);
  adv.loadAttribute("sampleProvided_", sampleProvided_);
  adv.loadAttribute("validationSampleProvided_", validationSampleProvided_);
  adv.loadAttribute("safeProbability_", safeProbability_);
  adv.loadAttribute("validationSafeProbability_", validationSafeProbability_);
  adv.loadAttribute("validationConfidenceInterval_", validationConfidenceInterval_);
  adv.loadAttribute("hasRun_", hasRun_);
}

} /* namespace OTROBOPT */
