//                                               -*- C++ -*-
/**
 *  @brief Worst-case measure evaluation
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
#include "otrobopt/WorstCaseMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/NLopt.hxx>
#include <openturns/TNC.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(WorstCaseMeasure);

static Factory<WorstCaseMeasure> RegisteredFactory;


/* Default constructor */
WorstCaseMeasure::WorstCaseMeasure()
  : MeasureEvaluationImplementation()
  , isMinimization_(true)
{
  // Nothing to do
}

/* Parameter constructor */
WorstCaseMeasure::WorstCaseMeasure (const NumericalMathFunction & function,
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


class WorstCaseMeasureParametricFunctionWrapper : public NumericalMathFunctionImplementation
{
public:
  WorstCaseMeasureParametricFunctionWrapper(const NumericalPoint & x,
                                            const NumericalMathFunction & function)
  : NumericalMathFunctionImplementation()
  , x_(x)
  , function_(function)
  {}

  virtual WorstCaseMeasureParametricFunctionWrapper * clone() const
  {
    return new WorstCaseMeasureParametricFunctionWrapper(*this);
  }

  NumericalPoint operator()(const NumericalPoint & theta) const
  {
    NumericalMathFunction function(function_);
    return function(x_, theta);
  }

  NumericalSample operator()(const NumericalSample & theta) const
  {
    NumericalMathFunction function(function_);
    return function(x_, theta);
  }

  Matrix gradient(const NumericalPoint & theta) const
  {
    NumericalMathFunction function(function_);
    return function.parameterGradient(x_, theta);
  }

  UnsignedInteger getInputDimension() const
  {
    return function_.getParameterDimension();
  }

  UnsignedInteger getOutputDimension() const
  {
    return 1;
  }

protected:
  NumericalPoint x_;
  NumericalMathFunction function_;
};




/* Evaluation */
NumericalPoint WorstCaseMeasure::operator()(const NumericalPoint & inP) const
{
  NumericalMathFunction function(getFunction());
  NumericalPoint parameter(function.getParameter());
  const UnsignedInteger outputDimension = function.getOutputDimension();
  NumericalPoint outP(outputDimension);
  if (getDistribution().isContinuous())
  {
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
    {
      Pointer<NumericalMathFunctionImplementation> p_wrapper(new WorstCaseMeasureParametricFunctionWrapper(inP, function.getMarginal(j)));
      const NumericalMathFunction G(p_wrapper);
      OptimizationProblem problem(G, NumericalMathFunction(), NumericalMathFunction(), getDistribution().getRange());
      problem.setMinimization(isMinimization());
      OptimizationSolver solver(solver_);
      solver.setStartingPoint(getDistribution().getMean());
      solver.setProblem(problem);
      solver.run();
      NumericalPoint optimalValue(solver.getResult().getOptimalValue());
      outP[j] = optimalValue[0];
    }
  }
  else
  {
    NumericalSample support(getDistribution().getSupport());
    const UnsignedInteger size = support.getSize();
    for (UnsignedInteger i = 0; i < size; ++ i)
    {
      NumericalPoint y(function(inP, support[i]));
      for (UnsignedInteger j = 0; j < outputDimension; ++ j)
      {
        if ((i == 0) || ((i > 0) && ((isMinimization_ && (y[j] < outP[j]))
                                  || (!isMinimization_ && (y[j] > outP[j])))))
        {
          outP[j] = y[j];
        }
      }
    }
  }
  function.setParameter(parameter);
  return outP;
}

/* Optimization solver accessor */
void WorstCaseMeasure::setOptimizationSolver(const OptimizationSolver & solver)
{
  solver_ = solver;
}

OptimizationSolver WorstCaseMeasure::getOptimizationSolver() const
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
