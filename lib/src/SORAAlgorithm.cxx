//                                               -*- C++ -*-
/**
 *  @brief Sequential Optimization and Reliability Analysis (SORA) for RBDO
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

#include "otrobopt/SORAAlgorithm.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/CompositeRandomVector.hxx>
#include <openturns/ThresholdEvent.hxx>
#include <openturns/BlockIndependentDistribution.hxx>
#include <openturns/Normal.hxx>
#include <openturns/NLopt.hxx>
#include <openturns/ParametricFunction.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(SORAAlgorithm)

static Factory<SORAAlgorithm> Factory_SORAAlgorithm;

/* Default constructor */
SORAAlgorithm::SORAAlgorithm()
  : RobustOptimizationAlgorithm()
  , maxSORAIteration_(40)
  , soraTolerance_(1e-3)
{
  // Nothing to do
}

/* Parameter constructor (RBDOProblem) */
SORAAlgorithm::SORAAlgorithm(const RBDOProblem & rbdoProblem,
                             const OptimizationAlgorithm & solver)
  : RobustOptimizationAlgorithm()
  , rbdoProblem_(rbdoProblem)
  , maxSORAIteration_(40)
  , soraTolerance_(1e-3)
{
  setOptimizationAlgorithm(solver);
}

void SORAAlgorithm::setStartingPoint(const Point & startingPoint)
{
  startingPoint_ = startingPoint;
}

Point SORAAlgorithm::getStartingPoint() const
{
  return startingPoint_;
}

/* Virtual constructor */
SORAAlgorithm * SORAAlgorithm::clone() const
{
  return new SORAAlgorithm(*this);
}

/* RBDO problem accessors */
void SORAAlgorithm::setRBDOProblem(const RBDOProblem & rbdoProblem)
{
  rbdoProblem_ = rbdoProblem;
}

RBDOProblem SORAAlgorithm::getRBDOProblem() const
{
  return rbdoProblem_;
}

/* SORA parameters accessors */
void SORAAlgorithm::setMaximumSORAIteration(UnsignedInteger maxIter)
{
  maxSORAIteration_ = maxIter;
}

UnsignedInteger SORAAlgorithm::getMaximumSORAIteration() const
{
  return maxSORAIteration_;
}

void SORAAlgorithm::setSORATolerance(Scalar tolerance)
{
  soraTolerance_ = tolerance;
}

Scalar SORAAlgorithm::getSORATolerance() const
{
  return soraTolerance_;
}

SORAAlgorithm::PointCollection SORAAlgorithm::getMostProbableTargetPoints() const
{
  return mostProbableTargetPoints_;
}

/* Run the SORA algorithm */
void SORAAlgorithm::run()
{
  const UnsignedInteger dimension = rbdoProblem_.getObjective().getInputDimension();
  Point currentPoint(dimension, 0.0);
  if (startingPoint_.getDimension() > 0)
    currentPoint = startingPoint_;

  mostProbableTargetPoints_.clear();
  const Indices activeDesignIndices(rbdoProblem_.getActiveDesignIndices());
  const Indices activeParameterIndices(rbdoProblem_.getActiveParameterIndices());

  Bool convergence = false;
  UnsignedInteger iteration = 0;

  while (!convergence && (iteration < maxSORAIteration_))
  {
    // --- Step 1: Deterministic optimization with shifted constraints ---
    OptimizationProblem problem;

    // Objective
    problem.setObjective(rbdoProblem_.getObjective());
    problem.setMinimization(true);

    // Bounds
    if (rbdoProblem_.getBounds().getDimension() > 0)
      problem.setBounds(rbdoProblem_.getBounds());

    // Build the shifted constraint evaluation
    // Evaluates g(d, x_MPTP, z_MPTP) where x_MPTP and z_MPTP
    // are the most probable target points from the previous iteration
    class ShiftedConstraintEvaluation : public EvaluationImplementation
    {
    public:
      ShiftedConstraintEvaluation(const RBDOProblem & rbdoProblem,
                                  const Point & xMPTP,
                                  const Point & zMPTP)
        : EvaluationImplementation()
        , rbdoProblem_(rbdoProblem)
        , xMPTP_(xMPTP)
        , zMPTP_(zMPTP)
      {
        const UnsignedInteger dim = rbdoProblem.getObjective().getInputDimension();
        setInputDescription(Description(dim));
        setOutputDescription(Description(1, "g_shifted"));
      }

      ShiftedConstraintEvaluation * clone() const override
      {
        return new ShiftedConstraintEvaluation(*this);
      }

      UnsignedInteger getInputDimension() const override
      {
        return rbdoProblem_.getObjective().getInputDimension();
      }

      UnsignedInteger getOutputDimension() const override
      {
        return 1;
      }

      Point operator()(const Point & designPoint) const override
      {
        Function g(rbdoProblem_.getConstraintFunction());
        const Indices activeDesignIndices(rbdoProblem_.getActiveDesignIndices());
        const Indices activeParameterIndices(rbdoProblem_.getActiveParameterIndices());
        Indices paramIndices(activeDesignIndices);
        paramIndices.add(activeParameterIndices);
        Point paramValues(paramIndices.getSize());
        for (UnsignedInteger i = 0; i < activeDesignIndices.getSize(); ++ i)
          paramValues[i] = designPoint[activeDesignIndices[i]];
        for (UnsignedInteger i = 0; i < activeParameterIndices.getSize(); ++ i)
          paramValues[activeDesignIndices.getSize() + i] = designPoint[activeParameterIndices[i]];
        const Function gParam(ParametricFunction(g, paramIndices, paramValues));

        const UnsignedInteger dimX = rbdoProblem_.getDistributionX().getDimension();
        const UnsignedInteger dimZ = rbdoProblem_.hasDistributionZ() ? rbdoProblem_.getDistributionZ().getDimension() : 0;
        const UnsignedInteger dimXZ = dimX + dimZ;

        Point xzInput(dimXZ);
        for (UnsignedInteger i = 0; i < dimX; ++ i)
          xzInput[i] = xMPTP_.getDimension() > i ? xMPTP_[i] : 0.0;
        for (UnsignedInteger i = 0; i < dimZ; ++ i)
          xzInput[dimX + i] = zMPTP_.getDimension() > i ? zMPTP_[i] : 0.0;

        Scalar gval = gParam(xzInput)[0];
        // For Less() -> feasible when gval >= 0 (return gval)
        // For Greater() -> feasible when gval <= 0 (return -gval)
        const Scalar sign = rbdoProblem_.getConstraintOperator()(1.0, 2.0) ? 1.0 : -1.0;
        return Point(1, sign * gval);
      }

    private:
      RBDOProblem rbdoProblem_;
      Point xMPTP_;
      Point zMPTP_;
    };

    // Set the shifted constraint using current MPTP points
    const Point xMPTP(mostProbableTargetPoints_.getSize() > 0 ? mostProbableTargetPoints_[0] : Point());
    const Point zMPTP((mostProbableTargetPoints_.getSize() > 1) ? mostProbableTargetPoints_[1] : Point());
    Pointer<EvaluationImplementation> p_eval(
      new ShiftedConstraintEvaluation(rbdoProblem_, xMPTP, zMPTP));
    const Function constraintFunc(*p_eval);
    problem.setInequalityConstraint(constraintFunc);

    // Solve the deterministic optimization
    OptimizationAlgorithm solver(solver_);
    solver.setProblem(problem);
    solver.setStartingPoint(currentPoint);
    solver.run();

    const OptimizationResult optResult(solver.getResult());
    const Point optimum(optResult.getOptimalPoint());

    // --- Step 2: Inverse FORM at the optimum to compute MPTP and shifting vector ---
    {
      Distribution jointDist(rbdoProblem_.buildJointDistribution());
      const UnsignedInteger dimXZ = jointDist.getDimension();
      const UnsignedInteger dimX = rbdoProblem_.getDistributionX().getDimension();
      const Scalar targetBeta = rbdoProblem_.getTargetBeta();

      Function g(rbdoProblem_.getConstraintFunction());
      Indices paramIndices(activeDesignIndices);
      paramIndices.add(activeParameterIndices);
      Point paramValues(paramIndices.getSize());
      for (UnsignedInteger i = 0; i < activeDesignIndices.getSize(); ++ i)
        paramValues[i] = optimum[activeDesignIndices[i]];
      for (UnsignedInteger i = 0; i < activeParameterIndices.getSize(); ++ i)
        paramValues[activeDesignIndices.getSize() + i] = optimum[activeParameterIndices[i]];
      const Function gParam(ParametricFunction(g, paramIndices, paramValues));

      const Function inverseTransform(jointDist.getInverseIsoProbabilisticTransformation());

      // Build the objective: minimize g in standard space
      class PMAObjective : public EvaluationImplementation
      {
      public:
        PMAObjective(const Function & gParam,
                     const Function & invTransform,
                     const ComparisonOperator & op)
          : EvaluationImplementation()
          , gParam_(gParam)
          , invTransform_(invTransform)
          , op_(op)
        {
          setInputDescription(Description(invTransform.getInputDimension()));
          setOutputDescription(Description(1, "g(u)"));
        }

        PMAObjective * clone() const override
        {
          return new PMAObjective(*this);
        }

        UnsignedInteger getInputDimension() const override
        {
          return invTransform_.getInputDimension();
        }

        UnsignedInteger getOutputDimension() const override
        {
          return 1;
        }

        Point operator()(const Point & u) const override
        {
          const Point xz(invTransform_(u));
          const Scalar gval = gParam_(xz)[0];
          const Scalar sign = op_(1.0, 2.0) ? 1.0 : -1.0;
          return Point(1, sign * gval);
        }

      private:
        Function gParam_;
        Function invTransform_;
        ComparisonOperator op_;
      };

      Pointer<EvaluationImplementation> p_pmaObj(new PMAObjective(
          gParam, inverseTransform, rbdoProblem_.getConstraintOperator()));
      Function objFunc(*p_pmaObj);

      // Equality constraint: ||u|| = beta_target
      class NormConstraint : public EvaluationImplementation
      {
      public:
        NormConstraint(UnsignedInteger dim, Scalar targetBeta)
          : EvaluationImplementation()
          , dim_(dim)
          , targetBeta_(targetBeta)
        {
          setInputDescription(Description(dim));
          setOutputDescription(Description(1, "||u|| - beta"));
        }

        NormConstraint * clone() const override
        {
          return new NormConstraint(*this);
        }

        UnsignedInteger getInputDimension() const override
        {
          return dim_;
        }

        UnsignedInteger getOutputDimension() const override
        {
          return 1;
        }

        Point operator()(const Point & u) const override
        {
          return Point(1, u.norm() - targetBeta_);
        }

      private:
        UnsignedInteger dim_;
        Scalar targetBeta_;
      };

      NLopt invAlgo("LD_SLSQP");
      OptimizationProblem invFormProb(objFunc);
      invFormProb.setMinimization(true);
      Pointer<EvaluationImplementation> p_norm(new NormConstraint(dimXZ, targetBeta));
      const Function normFunc(*p_norm);
      invFormProb.setEqualityConstraint(normFunc);
      invFormProb.setBounds(Interval(Point(dimXZ, -7.0), Point(dimXZ, 7.0)));
      invAlgo.setProblem(invFormProb);
      invAlgo.setStartingPoint(Point(dimXZ, 1.0));
      invAlgo.setMaximumIterationNumber(100);
      invAlgo.run();

      if (invAlgo.getResult().getStatus() != OptimizationResult::SUCCESS)
        throw InternalException(HERE) << "Inverse FORM optimization did not converge";

      const Point uMPTP(invAlgo.getResult().getOptimalPoint());
      const Point xzMPTP(inverseTransform(uMPTP));

      // Store MPTP points
      mostProbableTargetPoints_.clear();
      // Store x_MPTP (first dimX components of xzMPTP)
      Point xMPTPNew(dimX);
      for (UnsignedInteger i = 0; i < dimX; ++ i)
        xMPTPNew[i] = xzMPTP[i];
      mostProbableTargetPoints_.add(xMPTPNew);
      // Store z_MPTP (remaining components)
      if (rbdoProblem_.hasDistributionZ())
      {
        const UnsignedInteger dimZDist = rbdoProblem_.getDistributionZ().getDimension();
        Point zMPTPNew(dimZDist);
        for (UnsignedInteger i = 0; i < dimZDist; ++ i)
          zMPTPNew[i] = xzMPTP[dimX + i];
        mostProbableTargetPoints_.add(zMPTPNew);
      }
    }

    // --- Check convergence ---
    const Scalar error = (optimum - currentPoint).norm();
    convergence = (iteration > 0) && (error < soraTolerance_);
    currentPoint = optimum;

    // Callbacks
    if (progressCallback_.first)
      progressCallback_.first((100.0 * (iteration + 1)) / maxSORAIteration_, progressCallback_.second);
    if (stopCallback_.first && stopCallback_.first(stopCallback_.second))
      break;

    ++ iteration;
  }

  // Set the final result
  OptimizationResult finalResult;
  finalResult.setOptimalPoint(currentPoint);
  const Point optimalValue(rbdoProblem_.getObjective()(currentPoint));
  finalResult.setOptimalValue(optimalValue);
  setResult(finalResult);
}

/* String converter */
String SORAAlgorithm::__repr__() const
{
  OSS oss;
  oss << "class=" << SORAAlgorithm::GetClassName()
      << " rbdoProblem=" << rbdoProblem_
      << " maxSORAIteration=" << maxSORAIteration_
      << " soraTolerance=" << soraTolerance_;
  return oss;
}

/* Method save */
void SORAAlgorithm::save(Advocate & adv) const
{
  RobustOptimizationAlgorithm::save(adv);
  adv.saveAttribute("rbdoProblem_", rbdoProblem_);
  adv.saveAttribute("startingPoint_", startingPoint_);
  adv.saveAttribute("maxSORAIteration_", maxSORAIteration_);
  adv.saveAttribute("soraTolerance_", soraTolerance_);
  adv.saveAttribute("mostProbableTargetPoints_", mostProbableTargetPoints_);
}

/* Method load */
void SORAAlgorithm::load(Advocate & adv)
{
  RobustOptimizationAlgorithm::load(adv);
  adv.loadAttribute("rbdoProblem_", rbdoProblem_);
  adv.loadAttribute("startingPoint_", startingPoint_);
  adv.loadAttribute("maxSORAIteration_", maxSORAIteration_);
  adv.loadAttribute("soraTolerance_", soraTolerance_);
  adv.loadAttribute("mostProbableTargetPoints_", mostProbableTargetPoints_);
}

} /* namespace OTROBOPT */
