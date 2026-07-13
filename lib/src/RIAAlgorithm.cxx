//                                               -*- C++
/**
 *  @brief Reliability Index Approach (RIA) for RBDO
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

#include "otrobopt/RIAAlgorithm.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/FORM.hxx>
#include <openturns/AbdoRackwitz.hxx>
#include <openturns/StandardEvent.hxx>
#include <openturns/ThresholdEvent.hxx>
#include <openturns/CompositeRandomVector.hxx>
#include <openturns/BlockIndependentDistribution.hxx>
#include <openturns/Normal.hxx>
#include <openturns/ParametricFunction.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(RIAAlgorithm)

static Factory<RIAAlgorithm> Factory_RIAAlgorithm;

/* Default constructor */
RIAAlgorithm::RIAAlgorithm()
  : RobustOptimizationAlgorithm()
{
  // Nothing to do
}

/* Parameter constructor (RBDOProblem) */
RIAAlgorithm::RIAAlgorithm(const RBDOProblem & rbdoProblem,
                           const OptimizationAlgorithm & solver)
  : RobustOptimizationAlgorithm()
  , rbdoProblem_(rbdoProblem)
{
  setOptimizationAlgorithm(solver);
}

void RIAAlgorithm::setStartingPoint(const Point & startingPoint)
{
  startingPoint_ = startingPoint;
}

Point RIAAlgorithm::getStartingPoint() const
{
  return startingPoint_;
}

/* Virtual constructor */
RIAAlgorithm * RIAAlgorithm::clone() const
{
  return new RIAAlgorithm(*this);
}

/* RBDO problem accessors */
void RIAAlgorithm::setRBDOProblem(const RBDOProblem & rbdoProblem)
{
  rbdoProblem_ = rbdoProblem;
}

RBDOProblem RIAAlgorithm::getRBDOProblem() const
{
  return rbdoProblem_;
}

/* Run the algorithm */
void RIAAlgorithm::run()
{
  // Build the optimization problem
  OptimizationProblem problem;

  // Objective
  problem.setObjective(rbdoProblem_.getObjective());
  problem.setMinimization(true);

  // Bounds
  if (rbdoProblem_.getBounds().getDimension() > 0)
    problem.setBounds(rbdoProblem_.getBounds());

  // Build the constraint evaluation function using FORM
  // This inner class wraps FORM-based reliability analysis as an EvaluationImplementation
  class FORMConstraintEvaluation : public EvaluationImplementation
  {
  public:
    FORMConstraintEvaluation(const RBDOProblem & rbdoProblem)
      : EvaluationImplementation()
      , rbdoProblem_(rbdoProblem)
    {
      const UnsignedInteger dim = rbdoProblem.getObjective().getInputDimension();
      setInputDescription(Description(dim));
      setOutputDescription(Description(1, "beta - beta_target"));
    }

    FORMConstraintEvaluation * clone() const override
    {
      return new FORMConstraintEvaluation(*this);
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
      // Build the joint distribution of (X, Z)
      Distribution jointDist(rbdoProblem_.buildJointDistribution());

      // Build the limit state function g(d, p, x, z) with d and p fixed
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

      // Build the event
      const RandomVector antecedent(jointDist);
      const CompositeRandomVector composite(gParam, antecedent);
      const ThresholdEvent event(composite, rbdoProblem_.getConstraintOperator(), 0.0);

      // Run FORM
      AbdoRackwitz formSolver;
      const Point startingPoint(jointDist.getDimension(), 1.0);
      formSolver.setStartingPoint(startingPoint);
      FORM form(formSolver, event);
      form.run();
      const FORMResult formResult(form.getResult());

      if (formResult.getOptimizationResult().getStatus() != OptimizationResult::SUCCESS)
        return Point(1, -rbdoProblem_.getTargetBeta());

      // Extract the reliability index
      // If probability > 0.5, beta is negative
      Scalar beta = formResult.getHasoferReliabilityIndex();
      if (formResult.getEventProbability() > 0.5)
        beta = -beta;

      // Return beta - beta_target (feasible when >= 0)
      return Point(1, beta - rbdoProblem_.getTargetBeta());
    }

  private:
    RBDOProblem rbdoProblem_;
  };

  // Set the constraint function
  Pointer<EvaluationImplementation> p_eval(new FORMConstraintEvaluation(rbdoProblem_));
  const Function constraintFunc(*p_eval);
  problem.setInequalityConstraint(constraintFunc);

  // Solve the optimization problem
  OptimizationAlgorithm solver(solver_);
  solver.setProblem(problem);
  if (startingPoint_.getDimension() > 0)
    solver.setStartingPoint(startingPoint_);
  solver.run();

  // Store the result
  setResult(solver.getResult());
}

/* String converter */
String RIAAlgorithm::__repr__() const
{
  OSS oss;
  oss << "class=" << RIAAlgorithm::GetClassName()
      << " rbdoProblem=" << rbdoProblem_;
  return oss;
}

/* Method save */
void RIAAlgorithm::save(Advocate & adv) const
{
  RobustOptimizationAlgorithm::save(adv);
  adv.saveAttribute("rbdoProblem_", rbdoProblem_);
  adv.saveAttribute("startingPoint_", startingPoint_);
}

/* Method load */
void RIAAlgorithm::load(Advocate & adv)
{
  RobustOptimizationAlgorithm::load(adv);
  adv.loadAttribute("rbdoProblem_", rbdoProblem_);
  adv.loadAttribute("startingPoint_", startingPoint_);
}

} /* namespace OTROBOPT */
