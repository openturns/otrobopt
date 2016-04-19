//                                               -*- C++ -*-
/**
 *  @brief Solve a robust optimization problem
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
#include "otrobopt/RobustOptimizationAlgorithm.hxx"
#include <otrobopt/MeasureFunctionImplementation.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/UserDefined.hxx>
#include <openturns/LHSExperiment.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(RobustOptimizationAlgorithm);

static Factory<RobustOptimizationAlgorithm> RegisteredFactory;


/* Default constructor */
RobustOptimizationAlgorithm::RobustOptimizationAlgorithm()
  : OptimizationSolverImplementation()
{
  // Nothing to do
}

/* Parameter constructor */
RobustOptimizationAlgorithm::RobustOptimizationAlgorithm (const RobustOptimizationProblem & problem,
                                                          const OptimizationSolver & solver)
  : OptimizationSolverImplementation(problem)
  , solver_(solver)
{
  setRobustProblem(problem);
}

/* Virtual constructor method */
RobustOptimizationAlgorithm * RobustOptimizationAlgorithm::clone() const
{
  return new RobustOptimizationAlgorithm(*this);
}

/* String converter */
String RobustOptimizationAlgorithm::__repr__() const
{
  OSS oss;
  oss << "class=" << RobustOptimizationAlgorithm::GetClassName();
  return oss;
}

/* Performs the actual checks. Must be overloaded by the actual optimisation algorithm */
void RobustOptimizationAlgorithm::checkProblem(const OptimizationProblem & problem) const
{
  if (problem.hasLevelFunction())
    throw InvalidArgumentException(HERE) << "Error: " << this->getClassName() << " does not support level-function optimization";
  if (problem.hasEqualityConstraint())
    throw InvalidArgumentException(HERE) << "Error: " << this->getClassName() << " does not support equality constraints";
}

/* Problem accessors */
void RobustOptimizationAlgorithm::setRobustProblem(const RobustOptimizationProblem & robustProblem)
{
  if (!robustProblem.hasRobustnessMeasure() && !robustProblem.hasReliabilityMeasure())
    throw InvalidArgumentException(HERE) << "The problem should have at least a robustness or a reliability measure";
  robustProblem_ = robustProblem;
  setProblem(robustProblem);
}

RobustOptimizationProblem RobustOptimizationAlgorithm::getRobustProblem() const
{
  return robustProblem_;
}


/* Solver accessors */
void RobustOptimizationAlgorithm::setOptimizationSolver(const OptimizationSolver & solver)
{
  solver_ = solver;
}

OptimizationSolver RobustOptimizationAlgorithm::getOptimizationSolver() const
{
  return solver_;
}


/* Method save() stores the object through the StorageManager */
void RobustOptimizationAlgorithm::save(Advocate & adv) const
{
  OptimizationSolverImplementation::save(adv);
  adv.saveAttribute("robustProblem_", robustProblem_);
  adv.saveAttribute("solver_", solver_);
}

/* Method load() reloads the object from the StorageManager */
void RobustOptimizationAlgorithm::load(Advocate & adv)
{
  OptimizationSolverImplementation::load(adv);
  adv.loadAttribute("robustProblem_", robustProblem_);
  adv.loadAttribute("solver_", solver_);
}

} /* namespace OTROBOPT */
