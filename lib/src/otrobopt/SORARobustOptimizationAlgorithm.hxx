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
#ifndef OTROBOPT_SORAROBUSTOPTIMIZATIONALGORITHM_HXX
#define OTROBOPT_SORAROBUSTOPTIMIZATIONALGORITHM_HXX

#include <openturns/OptimizationAlgorithm.hxx>
#include <openturns/OptimizationAlgorithmImplementation.hxx>

#include "otrobopt/OTRobOptprivate.hxx"
#include "otrobopt/RBDOProblem.hxx"

namespace OTROBOPT
{

/**
 * @class SORARobustOptimizationAlgorithm
 *
 * Solve a reliability-based design optimization problem with a deterministic
 * objective and a joint chance constraint by sequential optimization and
 * reliability assessment (SORA).
 *
 * The algorithm alternates two steps.  Given a design point x^k and the
 * physical image theta*^k of the most probable point obtained by a
 * performance-measure analysis (PMA), it first solves the deterministic
 * problem
 *
 *   min f(x)  subject to  g(x; x^k - d^k) >= 0,
 *
 * where d^k = x^k - theta*^k is the physical shift.  It then performs a new
 * PMA at the resulting design, updates the shift and iterates until both the
 * design and the shift stabilize.
 *
 * The physical shift is only defined when the design dimension and the
 * random dimension are equal.  Problems with different dimensions are
 * rejected: they must be solved with DoubleLoopRBDOAlgorithm or with a
 * sample-based algorithm.
 *
 * A joint chance constraint is treated with one of two explicit
 * approximations:
 *
 * - EXACT: one scalarized system limit state sigma min_j g_j is used for the
 *   reliability analysis and one shift is maintained.  This is a FORM
 *   approximation of the system event, not an exact system reliability
 *   calculation.
 * - BONFERRONI: the joint event is replaced by the conservative componentwise
 *   targets 1 - (1 - alpha) / n_g.  Each component has its own PMA analysis
 *   and its own shift.
 *
 * References:
 *
 * Y. A. Tu, K. K. Choi and Y. H. Park, "A new study on reliability-based
 * design optimization", Journal of Mechanical Design, 121(4):557-564, 1999.
 * Introduces the performance-measure formulation of the inner reliability
 * problem used here.
 *
 * X. Du and W. Chen, "Sequential optimization and reliability assessment
 * method for efficient probabilistic design", Journal of Mechanical Design,
 * 126(2):225-233, 2004.  Introduces the sequence of deterministic
 * subproblems with a shift built from the previous target point, which is the
 * construction implemented here.
 *
 * A. Aoues and D. Chateauneuf, "Benchmark study of numerical methods for
 * reliability-based design optimization", Structural and Multidisciplinary
 * Optimization, 41(2):277-294, 2010.  Compares SORA with the double-loop and
 * single-loop formulations on standard RBDO benchmarks.
 *
 * M. A. Valdebenito and G. I. Schueller, "A survey on approaches for
 * reliability-based optimization", Structural and Multidisciplinary
 * Optimization, 42(5):645-663, 2010.  Surveys the decoupled and sequential
 * formulations.
 */
class OTROBOPT_API SORARobustOptimizationAlgorithm
  : public OT::OptimizationAlgorithmImplementation
{
  CLASSNAME

public:
  /** Treatment of a joint chance constraint */
  enum JointChanceTreatment
  {
    EXACT,
    BONFERRONI
  };

  /** Default constructor */
  SORARobustOptimizationAlgorithm();

  /** Parameter constructor */
  SORARobustOptimizationAlgorithm(const RBDOProblem & problem,
                                  const OT::OptimizationAlgorithm & solver);

  /** Parameter constructor */
  SORARobustOptimizationAlgorithm(const RBDOProblem & problem,
                                  const OT::OptimizationAlgorithm & solver,
                                  const OT::OptimizationAlgorithm & innerSolver);

  /** Virtual constructor method */
  SORARobustOptimizationAlgorithm * clone() const override;

  /** Performs the actual computation */
  void run() override;

  /** Problem accessor */
  void setProblem(const OT::OptimizationProblem & problem) override;
  void setRBDOProblem(const RBDOProblem & problem);
  RBDOProblem getRBDOProblem() const;

  /** Outer deterministic solver accessors */
  void setOptimizationAlgorithm(const OT::OptimizationAlgorithm & solver);
  OT::OptimizationAlgorithm getOptimizationAlgorithm() const;

  /** Inner reliability solver accessors */
  void setInnerOptimizationAlgorithm(const OT::OptimizationAlgorithm & solver);
  OT::OptimizationAlgorithm getInnerOptimizationAlgorithm() const;

  /** Joint chance treatment accessors */
  void setJointChanceTreatment(JointChanceTreatment treatment);
  JointChanceTreatment getJointChanceTreatment() const;

  /** Target reliability index defined by the joint chance constraint */
  OT::Scalar getTargetReliabilityIndex() const;

  /** Diagnostic reliability index at the solution */
  OT::Scalar getReliabilityIndex() const;

  /** Performance measure at the solution */
  OT::Scalar getPerformanceMeasure() const;

  /** Physical shift of the worst component at the solution */
  OT::Point getShift() const;

  /** Standard-space design point of the worst component at the solution */
  OT::Point getStandardSpaceDesignPoint() const;

  /** Physical-space design point of the worst component at the solution */
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
  JointChanceTreatment jointChanceTreatment_ = EXACT;
  OT::OptimizationAlgorithm solver_;
  OT::OptimizationAlgorithm innerSolver_;

  OT::Scalar reliabilityIndex_ = 0.0;
  OT::Scalar performanceMeasure_ = 0.0;
  OT::Scalar approximateProbability_ = 0.0;
  OT::Point shift_;
  OT::Point standardSpaceDesignPoint_;
  OT::Point physicalSpaceDesignPoint_;
  OT::Bool hasRun_ = false;

}; /* class SORARobustOptimizationAlgorithm */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_SORAROBUSTOPTIMIZATIONALGORITHM_HXX */
