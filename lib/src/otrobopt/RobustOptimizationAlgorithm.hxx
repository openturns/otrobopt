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
#ifndef OTROBOPT_ROBUSTOPTIMIZATIONALGORITHM_HXX
#define OTROBOPT_ROBUSTOPTIMIZATIONALGORITHM_HXX

#include <otrobopt/RobustOptimizationProblem.hxx>
#include <openturns/OptimizationSolverImplementation.hxx>
#include <openturns/OptimizationSolver.hxx>

namespace OTROBOPT
{

/**
 * @class RobustOptimizationAlgorithm
 *
 * Solve a robust optimization problem
 */
class OTROBOPT_API RobustOptimizationAlgorithm
  : public OT::OptimizationSolverImplementation
{
  CLASSNAME;

public:
  /** Default constructor */
  RobustOptimizationAlgorithm();

  /** Parameter constructor */
  RobustOptimizationAlgorithm(const RobustOptimizationProblem & problem,
                              const OT::OptimizationSolver & solver);

  /** Virtual constructor method */
  RobustOptimizationAlgorithm * clone() const;

  /** Problem accessors */
  void setRobustProblem(const RobustOptimizationProblem & problem);
  RobustOptimizationProblem getRobustProblem() const;

  /** Solver accessor */
  void setOptimizationSolver(const OT::OptimizationSolver & solver);
  OT::OptimizationSolver getOptimizationSolver() const;

  /** String converter */
  OT::String __repr__() const;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv);

protected:
  /** Check whether this problem can be solved by this solver.  Must be overloaded by the actual optimisation algorithm */
  virtual void checkProblem(const OT::OptimizationProblem & problem) const;

  RobustOptimizationProblem robustProblem_;
  OT::OptimizationSolver solver_;

}; /* class RobustOptimizationAlgorithm */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_ROBUSTOPTIMIZATIONALGORITHM_HXX */
