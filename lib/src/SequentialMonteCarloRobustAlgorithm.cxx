//                                               -*- C++ -*-
/**
 *  @brief Alternating discretization of measures and deterministic optimization steps
 *
 *  Copyright 2005-2024 Airbus-EDF-IMACS-Phimeca
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
#include "otrobopt/SequentialMonteCarloRobustAlgorithm.hxx"
#include <otrobopt/MeasureFactory.hxx>

#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/FixedExperiment.hxx>
#include <openturns/IdentityFunction.hxx>
#include <openturns/JointDistribution.hxx>
#include <openturns/LowDiscrepancyExperiment.hxx>
#include <openturns/SobolSequence.hxx>
#include <openturns/SpecFunc.hxx>
#include <openturns/Uniform.hxx>
#include <openturns/MultiStart.hxx>


using namespace OT;

namespace OT
{
TEMPLATE_CLASSNAMEINIT(PersistentCollection<OptimizationResult>)
}

namespace OTROBOPT
{

static const Factory<PersistentCollection<OptimizationResult> > Factory_PersistentCollection_OptimizationResult;

CLASSNAMEINIT(SequentialMonteCarloRobustAlgorithm)

static Factory<SequentialMonteCarloRobustAlgorithm> Factory_SequentialMonteCarloRobustAlgorithm;


/* Default constructor */
SequentialMonteCarloRobustAlgorithm::SequentialMonteCarloRobustAlgorithm()
  : RobustOptimizationAlgorithm()
  , initialSamplingSize_(ResourceMap::GetAsUnsignedInteger("SequentialMonteCarloRobustAlgorithm-DefaultInitialSamplingSize"))
  , samplingSizeIncrement_(IdentityFunction(1))
  , initialSearch_(0)
  , resultCollection_(0)
{
  // Nothing to do
}

/* Parameter constructor */
SequentialMonteCarloRobustAlgorithm::SequentialMonteCarloRobustAlgorithm (const RobustOptimizationProblem & problem,
    const OptimizationAlgorithm & solver)
  : RobustOptimizationAlgorithm(problem, solver)
  , initialSamplingSize_(ResourceMap::GetAsUnsignedInteger("SequentialMonteCarloRobustAlgorithm-DefaultInitialSamplingSize"))
  , samplingSizeIncrement_(IdentityFunction(1))
  , initialSearch_(0)
  , resultCollection_(0)
{
  // Nothing to do
}

/* Virtual constructor method */
SequentialMonteCarloRobustAlgorithm * SequentialMonteCarloRobustAlgorithm::clone() const
{
  return new SequentialMonteCarloRobustAlgorithm(*this);
}

/* Solve an optimization problem using a local multistart */
OptimizationResult SequentialMonteCarloRobustAlgorithm::doMultiStart(const OptimizationAlgorithm & solver, const Interval & bounds, Sample & initialStartingPoints) const
{
  LOGINFO("Start multistart search");
  LOGINFO(OSS() << "Generate Sobol' experiment with " << initialSearch_ << " points in bounds=" << bounds);
  const UnsignedInteger dimension = bounds.getDimension();
  JointDistribution::DistributionCollection coll(dimension);
  for (UnsignedInteger j = 0; j < dimension; ++ j)
    coll[j] = Uniform(bounds.getLowerBound()[j], bounds.getUpperBound()[j]);
  LowDiscrepancyExperiment initialExperiment(SobolSequence(), JointDistribution(coll), initialSearch_);
  initialStartingPoints = initialExperiment.generate();
  
  MultiStart multiStart(solver, initialStartingPoints);
  multiStart.run();
  return multiStart.getResult();
}

/* Evaluation */
void SequentialMonteCarloRobustAlgorithm::run()
{
  const UnsignedInteger dimension = getProblem().getObjective().getInputDimension();
  const UnsignedInteger outputDimension = getProblem().getObjective().getOutputDimension();

  const RobustOptimizationProblem robustProblem(getRobustProblem());

  // The distribution of the parameters can come from either
  Distribution distributionXi(robustProblem.getDistribution());

  Sample currentSampleXi(0, distributionXi.getDimension());
  UnsignedInteger N = initialSamplingSize_;

  Point currentPoint(dimension);
  Point currentValue(outputDimension);

  Bool convergence = false;

  // reset attributes
  setResult(OptimizationResult(robustProblem));
  resultCollection_.clear();
  initialStartingPoints_ = Sample(0, getProblem().getObjective().getInputDimension());

  UnsignedInteger iterationNumber = 0;
  LOGINFO("Start main loop");
  while ((!convergence) && (iterationNumber <= getMaximumIterationNumber()))
  {
    const UnsignedInteger increment = samplingSizeIncrement_(Point(1, N))[0];
    if (increment == 0) throw InvalidArgumentException(HERE) << "Increment must be positive";
    currentSampleXi.add(distributionXi.getSample(increment));
    N = currentSampleXi.getSize();
    LOGINFO(OSS() << "Iteration=" << iterationNumber << ", current increment=" << increment << ", current sample size=" << N);

    OptimizationProblem problem(getProblem());

    if (robustProblem.hasRobustnessMeasure())
    {
      LOGINFO(OSS() << "Discretize robustness measure=" << robustProblem.getRobustnessMeasure());
      // discretize the robustness measure
      const FixedExperiment experiment(currentSampleXi);
      const MeasureFactory robustnessFactory(experiment);
      MeasureEvaluation rhoJ = robustnessFactory.build(robustProblem.getRobustnessMeasure());
      problem.setObjective(*rhoJ.getImplementation());
    }

    if (robustProblem.hasReliabilityMeasure())
    {
      LOGINFO(OSS() << "Discretize reliability measure=" << robustProblem.getReliabilityMeasure());
      // discretize the reliability measure
      const FixedExperiment experiment(currentSampleXi);
      const MeasureFactory reliabilityFactory(experiment);
      MeasureEvaluation pG(reliabilityFactory.build(robustProblem.getReliabilityMeasure()));
      problem.setInequalityConstraint(*pG.getImplementation());
    }

    OptimizationAlgorithm solver(solver_);
    solver.setProblem(problem);

    const Scalar epsilon = ResourceMap::GetAsScalar("SequentialMonteCarloRobustAlgorithm-ConvergenceFactor") / std::sqrt(1.0 * N);
    solver.setMaximumResidualError(epsilon);
    solver.setMaximumConstraintError(epsilon);
    LOGINFO(OSS() << "solve the problem");

    Point newPoint;
    Point newValue;
    OptimizationResult result;
    if ((iterationNumber == 0) && (initialSearch_ > 0)) // multi-start
    {
      if (!getProblem().hasBounds())
        throw InvalidArgumentException(HERE) << "Cannot perform multi-start without bounds";
      result = doMultiStart(solver, problem.getBounds(), initialStartingPoints_);
    }
    else
    {
      if (iterationNumber == 0)
      {
        currentPoint = solver_.getStartingPoint();
      }
      solver.setStartingPoint(currentPoint);
      try
      {
	solver.run();
	result = solver.getResult();
	if (result.getOptimalPoint().getDimension() == 0) throw InvalidArgumentException(HERE) << "Local optimization failed, try multistart if allowed";
      }
      catch (const InvalidArgumentException & ex)
      {
	if (initialSearch_ > 0) // multi-start
	{
	  // Create tight neigbourhood of the current optimal point
	  Point lowerBound(currentPoint);
	  Point upperBound(currentPoint);
	  const Scalar delta = 3.0 / std::sqrt(N);
	  for (UnsignedInteger i = 0; i < dimension; ++i)
	  {
	    lowerBound[i] -= delta;
	    upperBound[i] += delta;
	  }
	  const Interval bounds(Interval(lowerBound, upperBound).intersect(problem.getBounds()));
	  Sample initialStartingPoints;
	  result = doMultiStart(solver, bounds, initialStartingPoints);
	} // initialSearch_ > 0
	// If no cure possible, rethrow the exception
	else throw;
      } // InvalidArgumentException
    } // (iterationNumber > 0) || (initialSearch_ == 0)
    resultCollection_.add(result);
    newPoint = result.getOptimalPoint();
    newValue = result.getOptimalValue();	

    LOGINFO(OSS() << "current optimum=" << newPoint);

    const Scalar absoluteError = (newPoint - currentPoint).norm();
    convergence = (iterationNumber > 0) && (absoluteError < getMaximumAbsoluteError());

    currentPoint = newPoint;
    currentValue = newValue;

    ++ iterationNumber;

    // update result
    result_.setIterationNumber(iterationNumber);
    result_.store(currentPoint, currentValue, absoluteError, 0.0, 0.0, 0.0);

    // callbacks
    if (progressCallback_.first)
    {
      progressCallback_.first((100.0 * iterationNumber) / getMaximumIterationNumber(), progressCallback_.second);
    }
    if (stopCallback_.first && stopCallback_.first(stopCallback_.second))
    {
      LOGWARN(OSS() << "Optimization was stopped by user");
      break;
    }
  }
  resultCollection_.add(result_);
}

/* Initial sampling size accessor */
void SequentialMonteCarloRobustAlgorithm::setInitialSamplingSize(const UnsignedInteger N0)
{
  initialSamplingSize_ = N0;
}

UnsignedInteger SequentialMonteCarloRobustAlgorithm::getInitialSamplingSize() const
{
  return initialSamplingSize_;
}

/* Discretization increment */
void SequentialMonteCarloRobustAlgorithm::setSamplingSizeIncrement(const Function & samplingSizeIncrement)
{
  samplingSizeIncrement_ = samplingSizeIncrement;
}

Function SequentialMonteCarloRobustAlgorithm::getSamplingSizeIncrement() const
{
  return samplingSizeIncrement_;
}

/* Number of initial starting points accessors */
void SequentialMonteCarloRobustAlgorithm::setInitialSearch(const UnsignedInteger initialSearch)
{
  initialSearch_ = initialSearch;
}

UnsignedInteger SequentialMonteCarloRobustAlgorithm::getInitialSearch() const
{
  return initialSearch_;
}

/* Optimization path accessor */
SequentialMonteCarloRobustAlgorithm::OptimizationResultCollection SequentialMonteCarloRobustAlgorithm::getResultCollection() const
{
  return resultCollection_;
}

Sample SequentialMonteCarloRobustAlgorithm::getInitialStartingPoints() const
{
  return initialStartingPoints_;
}

/* String converter */
String SequentialMonteCarloRobustAlgorithm::__repr__() const
{
  OSS oss;
  oss << "class=" << SequentialMonteCarloRobustAlgorithm::GetClassName()
      << ", initialSamplingSize=" << initialSamplingSize_
      << ", initialSearch=" << initialSearch_
      << ", resultCollection=" << resultCollection_
      << ", initialStartingPoints=" << initialStartingPoints_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void SequentialMonteCarloRobustAlgorithm::save(Advocate & adv) const
{
  RobustOptimizationAlgorithm::save(adv);
  adv.saveAttribute("initialSamplingSize_", initialSamplingSize_);
  adv.saveAttribute("initialSearch_", initialSearch_);
  adv.saveAttribute("resultCollection_", resultCollection_);
  adv.saveAttribute("initialStartingPoints_", initialStartingPoints_);
}

/* Method load() reloads the object from the StorageManager */
void SequentialMonteCarloRobustAlgorithm::load(Advocate & adv)
{
  RobustOptimizationAlgorithm::load(adv);
  adv.loadAttribute("initialSamplingSize_", initialSamplingSize_);
  adv.loadAttribute("initialSearch_", initialSearch_);
  adv.loadAttribute("resultCollection_", resultCollection_);
  adv.loadAttribute("initialStartingPoints_", initialStartingPoints_);
}


} /* namespace OTROBOPT */

