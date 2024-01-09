//                                               -*- C++ -*-
/**
 *  @brief Solve a robust optimization problem
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
#ifndef OTROBOPT_ROBUSTOPTIMIZATIONALGORITHM_HXX
#define OTROBOPT_ROBUSTOPTIMIZATIONALGORITHM_HXX

#include <otrobopt/RobustOptimizationProblem.hxx>
#include <openturns/OptimizationAlgorithmImplementation.hxx>
#include <openturns/OptimizationAlgorithm.hxx>

namespace OTROBOPT
{

/**
 * @class RobustOptimizationAlgorithm
 *
 * Solve a robust optimization problem
 */
class OTROBOPT_API RobustOptimizationAlgorithm
  : public OT::OptimizationAlgorithmImplementation
{
  CLASSNAME

public:
  /** Default constructor */
  RobustOptimizationAlgorithm();

  /** Parameter constructor */
  RobustOptimizationAlgorithm(const RobustOptimizationProblem & problem,
                              const OT::OptimizationAlgorithm & solver);

  /** Virtual constructor method */
  RobustOptimizationAlgorithm * clone() const override;

  /** Problem accessors */
  void setRobustProblem(const RobustOptimizationProblem & problem);
  RobustOptimizationProblem getRobustProblem() const;

  /** Solver accessor */
  void setOptimizationAlgorithm(const OT::OptimizationAlgorithm & solver);
  OT::OptimizationAlgorithm getOptimizationAlgorithm() const;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

protected:
  /** Check whether this problem can be solved by this solver.  Must be overloaded by the actual optimisation algorithm */
  void checkProblem(const OT::OptimizationProblem & problem) const override;

  RobustOptimizationProblem robustProblem_;
  OT::OptimizationAlgorithm solver_;

}; /* class RobustOptimizationAlgorithm */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_ROBUSTOPTIMIZATIONALGORITHM_HXX */
