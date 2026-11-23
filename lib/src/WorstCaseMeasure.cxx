//                                               -*- C++ -*-
/**
 *  @brief Worst-case measure evaluation
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
#include "otrobopt/WorstCaseMeasure.hxx"
#include "otrobopt/MeasureEvaluation.hxx"
#include <algorithm>
#include <cmath>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/NLopt.hxx>
#include <openturns/TNC.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(WorstCaseMeasure)

static Factory<WorstCaseMeasure> Factory_WorstCaseMeasure;

/* Build a finite starting point inside the optimization domain.
   The mean is the natural starting point, but it does not exist for every
   distribution and it may be infinite: fall back to the midpoint of the box
   then, as the canonicalizer does for its reference distribution. */
static Point buildStartingPoint(const Distribution & distribution,
                                const Interval & domain)
{
  const UnsignedInteger dimension = domain.getDimension();
  Point startingPoint(dimension);
  Bool useMean = false;
  try
  {
    const Point mean(distribution.getMean());
    if (mean.getDimension() == dimension)
    {
      useMean = true;
      for (UnsignedInteger i = 0; i < dimension; ++i)
        if (!std::isfinite(mean[i]))
          useMean = false;
      if (useMean)
        startingPoint = mean;
    }
  }
  catch (const Exception &)
  {
    // The distribution has no mean, use the box instead
  }

  const Interval::BoolCollection finiteLower(domain.getFiniteLowerBound());
  const Interval::BoolCollection finiteUpper(domain.getFiniteUpperBound());
  const Point lowerBound(domain.getLowerBound());
  const Point upperBound(domain.getUpperBound());
  for (UnsignedInteger i = 0; i < dimension; ++i)
  {
    if (useMean)
    {
      // Keep the mean inside the box
      if (finiteLower[i])
        startingPoint[i] = std::max(startingPoint[i], lowerBound[i]);
      if (finiteUpper[i])
        startingPoint[i] = std::min(startingPoint[i], upperBound[i]);
    }
    else if (finiteLower[i] && finiteUpper[i])
      startingPoint[i] = 0.5 * (lowerBound[i] + upperBound[i]);
    else if (finiteLower[i])
      startingPoint[i] = lowerBound[i] + 1.0;
    else if (finiteUpper[i])
      startingPoint[i] = upperBound[i] - 1.0;
    else
      startingPoint[i] = 0.0;
  }
  return startingPoint;
}


/* Default constructor */
WorstCaseMeasure::WorstCaseMeasure()
  : MeasureEvaluationImplementation()
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

/* Rebuild the measure over a new function and distribution */
MeasureEvaluation WorstCaseMeasure::rebuild(const Function & function,
    const Distribution & distribution,
    const Interval & optimizationDomain) const
{
  WorstCaseMeasure result(function, distribution, isMinimization_);
  result.setOptimizationAlgorithm(solver_);
  result.setIntegrationAlgorithm(getIntegrationAlgorithm());
  result.setPDFThreshold(getPDFThreshold());
  if (optimizationDomain.getDimension() != 0)
    result.setOptimizationDomain(optimizationDomain);
  else if (distribution == distribution_)
    result.setOptimizationDomain(optimizationDomain_);
  return result;
}

/* Whether rebuilding the measure requires an explicit optimization domain */
Bool WorstCaseMeasure::requiresOptimizationDomain() const
{
  return true;
}

/* Optimization domain accessor */
void WorstCaseMeasure::setOptimizationDomain(const Interval & domain)
{
  const UnsignedInteger dimension = distribution_.getDimension();
  if ((domain.getDimension() != 0) && (domain.getDimension() != dimension))
    throw InvalidDimensionException(HERE) << "The optimization domain dimension ("
                                          << domain.getDimension()
                                          << ") must match the distribution dimension ("
                                          << dimension << ")";
  optimizationDomain_ = domain;
}

Interval WorstCaseMeasure::getOptimizationDomain() const
{
  return optimizationDomain_;
}

/* Distribution accessor */
void WorstCaseMeasure::setDistribution(const Distribution & distribution)
{
  if ((optimizationDomain_.getDimension() != 0)
      && (optimizationDomain_.getDimension() != distribution.getDimension()))
    throw InvalidDimensionException(HERE) << "The optimization domain dimension ("
                                          << optimizationDomain_.getDimension()
                                          << ") must match the distribution dimension ("
                                          << distribution.getDimension() << ")";
  MeasureEvaluationImplementation::setDistribution(distribution);
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

  virtual WorstCaseMeasureParametricFunctionWrapper * clone() const override
  {
    return new WorstCaseMeasureParametricFunctionWrapper(*this);
  }

  Point operator()(const Point & theta) const override
  {
    Function function(function_);
    function.setParameter(theta);
    return function(x_);
  }

  Sample operator()(const Sample & theta) const override
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

  Matrix gradient(const Point & theta) const override
  {
    Function function(function_);
    function.setParameter(theta);
    return function.parameterGradient(x_);
  }

  UnsignedInteger getInputDimension() const override
  {
    return function_.getParameterDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return 1;
  }

  Description getInputDescription() const override
  {
    return function_.getParameterDescription();
  }

  Description getOutputDescription() const override
  {
    return function_.getOutputDescription();
  }

protected:
  Point x_;
  Function function_;
};

// The constraint on the PDF is defined on a log-scale for numerical stability
class WorstCaseMeasureLogPDFWrapper : public FunctionImplementation
{
public:
  WorstCaseMeasureLogPDFWrapper(const Distribution & distribution,
                                const Scalar logPDFThreshold)
    : FunctionImplementation()
    , distribution_(distribution)
    , logPDFThreshold_(logPDFThreshold)
  {
    // Nothing to do
  }

  virtual WorstCaseMeasureLogPDFWrapper * clone() const override
  {
    return new WorstCaseMeasureLogPDFWrapper(*this);
  }

  Point operator()(const Point & theta) const override
  {
    return Point(1, distribution_.computeLogPDF(theta));
  }

  Sample operator()(const Sample & theta) const override
  {
    return distribution_.computeLogPDF(theta);
  }

  Matrix gradient(const Point & theta) const override
  {
    const Scalar logPDF(distribution_.computeLogPDF(theta));
    const UnsignedInteger dimension = distribution_.getDimension();
    Point result(dimension);
    if (logPDF > logPDFThreshold_) result = distribution_.computeDDF(theta) * std::exp(-logPDF);
    return Matrix(dimension, 1, result);
  }

  UnsignedInteger getInputDimension() const override
  {
    return distribution_.getDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return 1;
  }

  Description getInputDescription() const override
  {
    return distribution_.getDescription();
  }

  Description getOutputDescription() const override
  {
    return Description(1, "logPDF");
  }

protected:
  Distribution distribution_;
  Scalar logPDFThreshold_;
};




/* Evaluation */
Point WorstCaseMeasure::operator()(const Point & inP) const
{
  Function function(getFunction());
  const UnsignedInteger outputDimension = function.getOutputDimension();
  Point outP(outputDimension);
  if (distribution_.isContinuous())
  {
    Function C;
    if (pdfThreshold_ > 0.0)
      {
        const Pointer<FunctionImplementation> p_wrapper(new WorstCaseMeasureLogPDFWrapper(distribution_, std::log(pdfThreshold_)));
        C = Function(p_wrapper);
      }
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
    {
      const Pointer<FunctionImplementation> p_wrapper(new WorstCaseMeasureParametricFunctionWrapper(inP, function.getMarginal(j)));
      const Function G(p_wrapper);
      Interval domain(optimizationDomain_);
      if (domain.getDimension() == 0)
        domain = distribution_.getRange();
      OptimizationProblem problem(G, Function(), C, domain);
      problem.setMinimization(isMinimization());
      OptimizationAlgorithm solver(solver_);
      solver.setStartingPoint(buildStartingPoint(distribution_, domain));
      solver.setProblem(problem);
      solver.run();
      const Point optimalValue(solver.getResult().getOptimalValue());
      outP[j] = optimalValue[0];
    } // j
  }
  else
  {
    const Point pdfs(distribution_.getProbabilities());
    const Sample parameters(distribution_.getSupport());
    const UnsignedInteger size = pdfs.getSize();
    const Bool hasDomain = (optimizationDomain_.getDimension() != 0);
    Sample values(0, outputDimension);
    for (UnsignedInteger i = 0; i < size; ++i)
    {
      if ((pdfs[i] > pdfThreshold_)
          && (!hasDomain || optimizationDomain_.contains(parameters[i])))
        {
          function.setParameter(parameters[i]);
          values.add(function(inP));
        }
    } // i
    if (values.getSize() == 0)
      throw InvalidArgumentException(HERE) << "No support point with a density above " << pdfThreshold_
                                           << " lies in the optimization domain";
    outP = (isMinimization_ ? values.getMin() : values.getMax());
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
void WorstCaseMeasure::setMinimization(const Bool minimization)
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
  if (optimizationDomain_.getDimension() != 0)
    oss << " optimizationDomain=" << optimizationDomain_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void WorstCaseMeasure::save(Advocate & adv) const
{
  MeasureEvaluationImplementation::save(adv);
  adv.saveAttribute("isMinimization_", isMinimization_);
  adv.saveAttribute("solver_", solver_);
  adv.saveAttribute("optimizationDomain_", optimizationDomain_);
}

/* Method load() reloads the object from the StorageManager */
void WorstCaseMeasure::load(Advocate & adv)
{
  MeasureEvaluationImplementation::load(adv);
  adv.loadAttribute("isMinimization_", isMinimization_);
  adv.loadAttribute("solver_", solver_);
  if (adv.hasAttribute("optimizationDomain_"))
    adv.loadAttribute("optimizationDomain_", optimizationDomain_);
}


} /* namespace OTROBOPT */
