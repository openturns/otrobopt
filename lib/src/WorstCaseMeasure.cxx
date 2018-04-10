//                                               -*- C++ -*-
/**
 *  @brief Worst-case measure evaluation
 *
 *  Copyright 2005-2018 Airbus-EDF-IMACS-Phimeca
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
#include "otrobopt/WorstCaseMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/NLopt.hxx>
#include <openturns/TNC.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(WorstCaseMeasure)

static Factory<WorstCaseMeasure> Factory_WorstCaseMeasure;


/* Default constructor */
WorstCaseMeasure::WorstCaseMeasure()
  : MeasureEvaluationImplementation()
  , isMinimization_(true)
{
  // Nothing to do
}

/* Parameter constructor */
WorstCaseMeasure::WorstCaseMeasure (const Function & function,
                                    const Distribution & distribution,
                                    const Bool minimization)
  : MeasureEvaluationImplementation(function, distribution)
  , isMinimization_(minimization)
  , solver_(new TNC())
{
  // Nothing to do
}

/* Virtual constructor method */
WorstCaseMeasure * WorstCaseMeasure::clone() const
{
  return new WorstCaseMeasure(*this);
}


class WorstCaseMeasureParametricFunctionWrapper : public FunctionImplementation
{
public:
  WorstCaseMeasureParametricFunctionWrapper(const Point & x,
      const Function & function)
    : FunctionImplementation()
    , x_(x)
    , function_(function)
  {
    // Nothing to do
  }

  virtual WorstCaseMeasureParametricFunctionWrapper * clone() const
  {
    return new WorstCaseMeasureParametricFunctionWrapper(*this);
  }

  Point operator()(const Point & theta) const
  {
    Function function(function_);
    function.setParameter(theta);
    return function(x_);
  }

  Sample operator()(const Sample & theta) const
  {
    Function function(function_);
    const UnsignedInteger size = theta.getSize();
    const UnsignedInteger outputDimension = function.getOutputDimension();
    Sample values(size, outputDimension);
    for (UnsignedInteger i = 0; i < size; ++i)
    {
      function.setParameter(theta[i]);
      values[i] = function(x_);
    }
    return values;
  }

  Matrix gradient(const Point & theta) const
  {
    Function function(function_);
    function.setParameter(theta);
    return function.parameterGradient(x_);
  }

  UnsignedInteger getInputDimension() const
  {
    return function_.getParameterDimension();
  }

  UnsignedInteger getOutputDimension() const
  {
    return 1;
  }

  Description getInputDescription() const
  {
    return function_.getParameterDescription();
  }

  Description getOutputDescription() const
  {
    return function_.getOutputDescription();
  }

protected:
  Point x_;
  Function function_;
};




/* Evaluation */
Point WorstCaseMeasure::operator()(const Point & inP) const
{
  Function function(getFunction());
  const UnsignedInteger outputDimension = function.getOutputDimension();
  Point outP(outputDimension);
  if (getDistribution().isContinuous())
  {
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
    {
      Pointer<FunctionImplementation> p_wrapper(new WorstCaseMeasureParametricFunctionWrapper(inP, function.getMarginal(j)));
      const Function G(p_wrapper);
      OptimizationProblem problem(G, Function(), Function(), getDistribution().getRange());
      problem.setMinimization(isMinimization());
      OptimizationAlgorithm solver(solver_);
      solver.setStartingPoint(getDistribution().getMean());
      solver.setProblem(problem);
      solver.run();
      Point optimalValue(solver.getResult().getOptimalValue());
      outP[j] = optimalValue[0];
    }
  }
  else
  {
    const Sample parameters(getDistribution().getSupport());
    const UnsignedInteger size = parameters.getSize();
    Sample values(size, outputDimension);
    for (UnsignedInteger i = 0; i < size; ++i)
    {
      function.setParameter(parameters[i]);
      values[i] = function(inP);
    }
    if (isMinimization_)
      outP = values.getMin();
    else
      outP = values.getMax();
  } // discrete
  return outP;
}

/* Optimization solver accessor */
void WorstCaseMeasure::setOptimizationAlgorithm(const OptimizationAlgorithm & solver)
{
  solver_ = solver;
}

OptimizationAlgorithm WorstCaseMeasure::getOptimizationAlgorithm() const
{
  return solver_;
}


/* Minimization accessor */
void WorstCaseMeasure::setMinimization(Bool minimization)
{
  isMinimization_ = minimization;
}

Bool WorstCaseMeasure::isMinimization() const
{
  return isMinimization_;
}

/* String converter */
String WorstCaseMeasure::__repr__() const
{
  OSS oss;
  oss << "class=" << WorstCaseMeasure::GetClassName()
      << " minimization=" << isMinimization_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void WorstCaseMeasure::save(Advocate & adv) const
{
  MeasureEvaluationImplementation::save(adv);
  adv.saveAttribute("isMinimization_", isMinimization_);
  adv.saveAttribute("solver_", solver_);
}

/* Method load() reloads the object from the StorageManager */
void WorstCaseMeasure::load(Advocate & adv)
{
  MeasureEvaluationImplementation::load(adv);
  adv.loadAttribute("isMinimization_", isMinimization_);
  adv.loadAttribute("solver_", solver_);
}


} /* namespace OTROBOPT */
