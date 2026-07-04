//                                               -*- C++ -*-
/**
 *  @brief Performance Measure Approach (PMA) for RBDO
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

#include "otrobopt/PMAAlgorithm.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/CompositeRandomVector.hxx>
#include <openturns/ThresholdEvent.hxx>
#include <openturns/BlockIndependentDistribution.hxx>
#include <openturns/Normal.hxx>
#include <openturns/OptimizationAlgorithm.hxx>
#include <openturns/ParametricFunction.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(PMAAlgorithm)

static Factory<PMAAlgorithm> Factory_PMAAlgorithm;

namespace
{

class PMAAlgorithm_SphereMinimizationObjective : public EvaluationImplementation
{
public:
  PMAAlgorithm_SphereMinimizationObjective(const Function & gParam,
                               const Function & invTransform,
                               const ComparisonOperator & op)
    : EvaluationImplementation()
    , gParam_(gParam)
    , invTransform_(invTransform)
    , op_(op)
  {
    // Nothing to do
  }

  PMAAlgorithm_SphereMinimizationObjective * clone() const override
  {
    return new PMAAlgorithm_SphereMinimizationObjective(*this);
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

class PMAAlgorithm_SphereConstraint : public EvaluationImplementation
{
public:
  PMAAlgorithm_SphereConstraint(UnsignedInteger dim, Scalar targetBeta)
    : EvaluationImplementation()
    , dim_(dim)
    , targetBeta_(targetBeta)
  {
    setInputDescription(Description(dim));
    setOutputDescription(Description(1, "||u|| - beta"));
  }

  PMAAlgorithm_SphereConstraint * clone() const override
  {
    return new PMAAlgorithm_SphereConstraint(*this);
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

class PMAAlgorithm_ConstraintEvaluation : public EvaluationImplementation
{
public:
  PMAAlgorithm_ConstraintEvaluation(const RBDOProblem & rbdoProblem)
    : EvaluationImplementation()
    , rbdoProblem_(rbdoProblem)
  {
    const UnsignedInteger dim = rbdoProblem.getObjective().getInputDimension();
    setInputDescription(Description(dim));
    setOutputDescription(Description(1, "-performance_measure"));
  }

  PMAAlgorithm_ConstraintEvaluation * clone() const override
  {
    return new PMAAlgorithm_ConstraintEvaluation(*this);
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
    Distribution jointDist(rbdoProblem_.buildJointDistribution());
    const UnsignedInteger dimXZ = jointDist.getDimension();

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

    const Scalar targetBeta = rbdoProblem_.getTargetBeta();
    const ComparisonOperator op(rbdoProblem_.getConstraintOperator());

    const Function inverseTransform(jointDist.getInverseIsoProbabilisticTransformation());

    const Function objFunc(PMAAlgorithm_SphereMinimizationObjective(gParam, inverseTransform, op));

    OptimizationProblem sphereMinimizationProblem(objFunc);
    sphereMinimizationProblem.setMinimization(true);
    const Function sphereFunc(PMAAlgorithm_SphereConstraint(dimXZ, targetBeta));
    sphereMinimizationProblem.setEqualityConstraint(sphereFunc);
    sphereMinimizationProblem.setBounds(Interval(Point(dimXZ, -7.0), Point(dimXZ, 7.0)));
    OptimizationAlgorithm algo(OptimizationAlgorithm::Build(sphereMinimizationProblem));
    if (uStart_.getDimension() != dimXZ)
      uStart_ = Point(dimXZ, 1.0);
    algo.setStartingPoint(uStart_);
    algo.setMaximumIterationNumber(100);
    algo.run();

    if (algo.getResult().getStatus() != OptimizationResult::SUCCESS)
      throw InternalException(HERE) << "Inverse FORM optimization did not converge";

    const Point uMPTP(algo.getResult().getOptimalPoint());
    uStart_ = uMPTP;

    const Point xzMPTP(inverseTransform(uMPTP));

    const Scalar gMPTP = gParam(xzMPTP)[0];

    const Scalar sign = op(1.0, 2.0) ? 1.0 : -1.0;
    return Point(1, sign * gMPTP);
  }

private:
  RBDOProblem rbdoProblem_;
  mutable Point uStart_;
};

} // anonymous namespace

/* Default constructor */
PMAAlgorithm::PMAAlgorithm()
  : RobustOptimizationAlgorithm()
{
  // Nothing to do
}

/* Parameter constructor (RBDOProblem) */
PMAAlgorithm::PMAAlgorithm(const RBDOProblem & rbdoProblem,
                           const OptimizationAlgorithm & solver)
  : RobustOptimizationAlgorithm()
  , rbdoProblem_(rbdoProblem)
{
  setOptimizationAlgorithm(solver);
}

void PMAAlgorithm::setStartingPoint(const Point & startingPoint)
{
  startingPoint_ = startingPoint;
}

Point PMAAlgorithm::getStartingPoint() const
{
  return startingPoint_;
}

/* Virtual constructor */
PMAAlgorithm * PMAAlgorithm::clone() const
{
  return new PMAAlgorithm(*this);
}

/* RBDO problem accessors */
void PMAAlgorithm::setRBDOProblem(const RBDOProblem & rbdoProblem)
{
  rbdoProblem_ = rbdoProblem;
}

RBDOProblem PMAAlgorithm::getRBDOProblem() const
{
  return rbdoProblem_;
}

/* Run the algorithm */
void PMAAlgorithm::run()
{
  OptimizationProblem problem;

  problem.setObjective(rbdoProblem_.getObjective());
  problem.setMinimization(true);

  if (rbdoProblem_.getBounds().getDimension() > 0)
    problem.setBounds(rbdoProblem_.getBounds());

  Pointer<EvaluationImplementation> p_eval(new PMAAlgorithm_ConstraintEvaluation(rbdoProblem_));
  const Function constraintFunc(*p_eval);
  problem.setInequalityConstraint(constraintFunc);

  OptimizationAlgorithm solver(solver_);
  solver.setProblem(problem);
  if (startingPoint_.getDimension() > 0)
    solver.setStartingPoint(startingPoint_);
  solver.run();

  setResult(solver.getResult());
}

/* String converter */
String PMAAlgorithm::__repr__() const
{
  OSS oss;
  oss << "class=" << PMAAlgorithm::GetClassName()
      << " rbdoProblem=" << rbdoProblem_;
  return oss;
}

/* Method save */
void PMAAlgorithm::save(Advocate & adv) const
{
  RobustOptimizationAlgorithm::save(adv);
  adv.saveAttribute("rbdoProblem_", rbdoProblem_);
  adv.saveAttribute("startingPoint_", startingPoint_);
}

/* Method load */
void PMAAlgorithm::load(Advocate & adv)
{
  RobustOptimizationAlgorithm::load(adv);
  adv.loadAttribute("rbdoProblem_", rbdoProblem_);
  adv.loadAttribute("startingPoint_", startingPoint_);
}

} /* namespace OTROBOPT */
