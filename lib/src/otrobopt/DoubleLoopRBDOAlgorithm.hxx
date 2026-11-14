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
#ifndef OTROBOPT_DOUBLELOOPRBDOALGORITHM_HXX
#define OTROBOPT_DOUBLELOOPRBDOALGORITHM_HXX

#include <openturns/OptimizationAlgorithm.hxx>
#include <openturns/OptimizationAlgorithmImplementation.hxx>

#include "otrobopt/OTRobOptprivate.hxx"
#include "otrobopt/RBDOProblem.hxx"

namespace OTROBOPT
{

/**
 * @class DoubleLoopRBDOAlgorithm
 *
 * Solve a reliability-based design optimization problem with an outer
 * deterministic optimization and an inner reliability analysis.
 *
 * The algorithm requires a fixed distribution parameter.  The inner
 * reliability analysis is performed in the standard normal space with a
 * FORM approximation.  Two formulations are available:
 *
 * - RIA minimizes the distance to the limit state and compares the signed
 *   reliability index with the target index.
 * - PMA minimizes the limit-state function on the sphere of radius equal to
 *   the target reliability index.
 *
 * The joint chance constraint is treated with one of two explicit
 * approximations: a single scalarized system limit state, or the conservative
 * Bonferroni split.  The treatment is selected explicitly and reported.
 */
class OTROBOPT_API DoubleLoopRBDOAlgorithm
  : public OT::OptimizationAlgorithmImplementation
{
  CLASSNAME

public:
  /** Formulation of the inner reliability analysis */
  enum Method
  {
    RIA,
    PMA
  };

  /** Treatment of a joint chance constraint */
  enum JointChanceTreatment
  {
    EXACT,
    BONFERRONI
  };

  /** Default constructor */
  DoubleLoopRBDOAlgorithm();

  /** Parameter constructor */
  DoubleLoopRBDOAlgorithm(const RBDOProblem & problem,
                          const OT::OptimizationAlgorithm & solver);

  /** Parameter constructor */
  DoubleLoopRBDOAlgorithm(const RBDOProblem & problem,
                          const OT::OptimizationAlgorithm & solver,
                          const OT::OptimizationAlgorithm & innerSolver);

  /** Virtual constructor method */
  DoubleLoopRBDOAlgorithm * clone() const override;

  /** Performs the actual computation */
  void run() override;

  /** Problem accessor */
  void setProblem(const OT::OptimizationProblem & problem) override;
  void setRBDOProblem(const RBDOProblem & problem);
  RBDOProblem getRBDOProblem() const;

  /** Outer optimization solver accessors */
  void setOptimizationAlgorithm(const OT::OptimizationAlgorithm & solver);
  OT::OptimizationAlgorithm getOptimizationAlgorithm() const;

  /** Inner reliability solver accessors */
  void setInnerOptimizationAlgorithm(const OT::OptimizationAlgorithm & solver);
  OT::OptimizationAlgorithm getInnerOptimizationAlgorithm() const;

  /** Inner formulation accessors */
  void setMethod(Method method);
  Method getMethod() const;

  /** Joint chance treatment accessors */
  void setJointChanceTreatment(JointChanceTreatment treatment);
  JointChanceTreatment getJointChanceTreatment() const;

  /** Target reliability index defined by the joint chance constraint */
  OT::Scalar getTargetReliabilityIndex() const;

  /** Signed reliability index at the solution */
  OT::Scalar getReliabilityIndex() const;

  /** Performance measure at the solution */
  OT::Scalar getPerformanceMeasure() const;

  /** Standard-space design point at the solution */
  OT::Point getStandardSpaceDesignPoint() const;

  /** Physical-space design point at the solution */
  OT::Point getPhysicalSpaceDesignPoint() const;

  /** FORM approximation of the safe probability at the solution */
  OT::Scalar getApproximateProbability() const;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

protected:
  /** Check whether this problem can be solved by this solver */
  void checkProblem(const OT::OptimizationProblem & problem) const override;

private:
  OT::Point buildStartingPoint() const;
  OT::Bool hasRun() const;

  RBDOProblem rbdoProblem_;
  Method method_ = PMA;
  JointChanceTreatment jointChanceTreatment_ = EXACT;
  OT::OptimizationAlgorithm solver_;
  OT::OptimizationAlgorithm innerSolver_;

  OT::Scalar reliabilityIndex_ = 0.0;
  OT::Scalar performanceMeasure_ = 0.0;
  OT::Scalar approximateProbability_ = 0.0;
  OT::Point standardSpaceDesignPoint_;
  OT::Point physicalSpaceDesignPoint_;
  OT::Bool hasRun_ = false;

}; /* class DoubleLoopRBDOAlgorithm */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_DOUBLELOOPRBDOALGORITHM_HXX */
