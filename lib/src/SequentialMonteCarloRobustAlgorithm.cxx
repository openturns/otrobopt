//                                               -*- C++ -*-
/**
 *  @brief Alternating discretization of measures and deterministic optimization steps
 *
 *  Copyright 2005-2016 Airbus-EDF-IMACS-Phimeca
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
#include "otrobopt/SequentialMonteCarloRobustAlgorithm.hxx"
#include <otrobopt/MeasureFactory.hxx>

#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/FixedExperiment.hxx>
#include <openturns/Normal.hxx>
#include <openturns/ComposedDistribution.hxx>
#include <openturns/LHSExperiment.hxx>
#include <openturns/SpecFunc.hxx>
#include <openturns/Uniform.hxx>


using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(SequentialMonteCarloRobustAlgorithm);

static Factory<SequentialMonteCarloRobustAlgorithm> RegisteredFactory;


/* Default constructor */
SequentialMonteCarloRobustAlgorithm::SequentialMonteCarloRobustAlgorithm()
  : RobustOptimizationAlgorithm()
  , initialSamplingSize_(ResourceMap::GetAsUnsignedInteger("SequentialMonteCarloRobustAlgorithm-DefaultInitialSamplingSize"))
  , initialSearch_(0)
{
  // Nothing to do
}

/* Parameter constructor */
SequentialMonteCarloRobustAlgorithm::SequentialMonteCarloRobustAlgorithm (const RobustOptimizationProblem & problem,
                                                                          const OptimizationSolver & solver)
  : RobustOptimizationAlgorithm(problem, solver)
  , initialSamplingSize_(ResourceMap::GetAsUnsignedInteger("SequentialMonteCarloRobustAlgorithm-DefaultInitialSamplingSize"))
  , initialSearch_(0)
{
  // Nothing to do
}

/* Virtual constructor method */
SequentialMonteCarloRobustAlgorithm * SequentialMonteCarloRobustAlgorithm::clone() const
{
  return new SequentialMonteCarloRobustAlgorithm(*this);
}


/* Evaluation */
void SequentialMonteCarloRobustAlgorithm::run()
{
  const UnsignedInteger dimension = getProblem().getObjective().getInputDimension();
  const UnsignedInteger outputDimension = getProblem().getObjective().getOutputDimension();

  const RobustOptimizationProblem robustProblem(getRobustProblem());

  // The distribution of the parameters can come from either
  Distribution distributionXi;
  if (robustProblem.hasRobustnessMeasure())
    distributionXi = robustProblem.getRobustnessMeasure().getDistribution();
  if (robustProblem.hasReliabilityMeasure())
    distributionXi = robustProblem.getReliabilityMeasure().getDistribution();

  NumericalSample currentSampleXi(0, distributionXi.getDimension());
  UnsignedInteger N = initialSamplingSize_;

  NumericalPoint currentPoint(dimension);
  NumericalPoint currentValue(outputDimension);

  Bool convergence = false;

  // reset result
  setResult(OptimizationResult(currentPoint, currentValue, 0, 0.0, 0.0, 0.0, 0.0));

  UnsignedInteger iterationNumber = 0;
  while ((!convergence) && (iterationNumber <= getMaximumIterationNumber()))
  {
    currentSampleXi.add(distributionXi.getSample(N));
    N = currentSampleXi.getSize();

    OptimizationProblem problem(getProblem());

    if (robustProblem.hasRobustnessMeasure())
    {
      // discretize the robustness measure
      const MeasureFactory robustnessFactory(robustProblem.getRobustnessMeasure(), FixedExperiment(currentSampleXi));
      MeasureEvaluation rhoJ(robustnessFactory.build());
      problem.setObjective(*rhoJ.getImplementation());
    }

    if (robustProblem.hasReliabilityMeasure())
    {
      // discretize the reliability measure
      const MeasureFactory reliabilityFactory(robustProblem.getReliabilityMeasure(), FixedExperiment(currentSampleXi));
      MeasureEvaluation pG(reliabilityFactory.build());
      problem.setInequalityConstraint(*pG.getImplementation());
    }

    OptimizationSolver solver(solver_);
    solver.setProblem(problem);

    const NumericalScalar epsilon = ResourceMap::GetAsNumericalScalar("SequentialMonteCarloRobustAlgorithm-ConvergenceFactor") / sqrt(1.0 * N);
    solver.setMaximumAbsoluteError(epsilon);
    solver.setMaximumRelativeError(epsilon);
    solver.setMaximumResidualError(epsilon);
    solver.setMaximumConstraintError(epsilon);
    LOGINFO(OSS() << "solve the problem");

    NumericalPoint newPoint;
    NumericalPoint newValue;
    if ((iterationNumber == 0) && (initialSearch_ > 0)) // multi-start
    {
      if (!getProblem().hasBounds())
        throw InvalidArgumentException(HERE) << "Cannot perform multi-start without bounds";

      ComposedDistribution::DistributionCollection coll(dimension);
      for (UnsignedInteger j = 0; j < dimension; ++ j)
      {
        coll[j] = Uniform(getProblem().getBounds().getLowerBound()[j], getProblem().getBounds().getUpperBound()[j]);
      }
      Distribution initialDistribution = ComposedDistribution(coll);

      LHSExperiment initialExperiment(initialDistribution, initialSearch_);
      NumericalSample startingPoints(initialExperiment.generate());
      NumericalScalar bestValue = getProblem().isMinimization() ? SpecFunc::MaxNumericalScalar : -SpecFunc::MaxNumericalScalar;
      for (UnsignedInteger i = 0; i < initialSearch_; ++ i)
      {
        solver.setStartingPoint(startingPoints[i]);
        solver.run();
        OptimizationResult result(solver.getResult());
        NumericalScalar currentValue0 = result.getOptimalValue()[0];
        if ((getProblem().isMinimization() && (currentValue0 < bestValue))
        || (!getProblem().isMinimization() && (currentValue0 > bestValue)))
        {
          bestValue = currentValue0;
          newPoint = result.getOptimalPoint();
          newValue = result.getOptimalValue();
          LOGINFO(OSS() << "Best initial point so far=" << newPoint << " value=" << bestValue);
        }
      }
    }
    else
    {
      if (iterationNumber == 0)
      {
        currentPoint = solver_.getStartingPoint();
      }
      solver.setStartingPoint(currentPoint);
      solver.run();
      OptimizationResult result(solver.getResult());
      newPoint = result.getOptimalPoint();
      newValue = result.getOptimalValue();
    }

    LOGINFO(OSS() << "current optimum=" << newPoint);

    const NumericalScalar absoluteError = (newPoint - currentPoint).norm();
    convergence = (iterationNumber > 0) && ((absoluteError < getMaximumAbsoluteError()) || (epsilon < getMaximumAbsoluteError()));

    currentPoint = newPoint;
    currentValue = newValue;

    // update result
    result_.update(currentPoint, iterationNumber);
    result_.store(currentPoint, currentValue, absoluteError, 0.0, 0.0, 0.0);

    ++ iterationNumber;
  }
}

/* Initial sampling size accessor */
void SequentialMonteCarloRobustAlgorithm::setInitialSamplingSize(const OT::UnsignedInteger N0)
{
  initialSamplingSize_ = N0;
}

UnsignedInteger SequentialMonteCarloRobustAlgorithm::getInitialSamplingSize() const
{
  return initialSamplingSize_;
}

void SequentialMonteCarloRobustAlgorithm::setInitialSearch(const OT::UnsignedInteger initialSearch)
{
  initialSearch_ = initialSearch;
}

UnsignedInteger SequentialMonteCarloRobustAlgorithm::getInitialSearch() const
{
  return initialSearch_;
}

/* String converter */
String SequentialMonteCarloRobustAlgorithm::__repr__() const
{
  OSS oss;
  oss << "class=" << SequentialMonteCarloRobustAlgorithm::GetClassName();
  return oss;
}

/* Method save() stores the object through the StorageManager */
void SequentialMonteCarloRobustAlgorithm::save(Advocate & adv) const
{
  RobustOptimizationAlgorithm::save(adv);
  adv.saveAttribute("initialSamplingSize_", initialSamplingSize_);
  adv.saveAttribute("initialSearch_", initialSearch_);
}

/* Method load() reloads the object from the StorageManager */
void SequentialMonteCarloRobustAlgorithm::load(Advocate & adv)
{
  RobustOptimizationAlgorithm::load(adv);
  adv.loadAttribute("initialSamplingSize_", initialSamplingSize_);
  adv.loadAttribute("initialSearch_", initialSearch_);
}


} /* namespace OTROBOPT */
