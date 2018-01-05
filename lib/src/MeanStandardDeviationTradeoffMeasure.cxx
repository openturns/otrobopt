//                                               -*- C++ -*-
/**
 *  @brief Tradeoff between mean and standard deviation
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
#include "otrobopt/MeanStandardDeviationTradeoffMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/IteratedQuadrature.hxx>
#include <openturns/UserDefined.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(MeanStandardDeviationTradeoffMeasure);

static Factory<MeanStandardDeviationTradeoffMeasure> Factory_MeanStandardDeviationTradeoffMeasure;


/* Default constructor */
MeanStandardDeviationTradeoffMeasure::MeanStandardDeviationTradeoffMeasure()
  : MeasureEvaluationImplementation()
{
  // Nothing to do
}

/* Parameter constructor */
MeanStandardDeviationTradeoffMeasure::MeanStandardDeviationTradeoffMeasure (const Function & function,
                                                                            const Distribution & distribution,
                                                                            const Point & alpha)
  : MeasureEvaluationImplementation(function, distribution)
{
  setAlpha(alpha);
}

/* Virtual constructor method */
MeanStandardDeviationTradeoffMeasure * MeanStandardDeviationTradeoffMeasure::clone() const
{
  return new MeanStandardDeviationTradeoffMeasure(*this);
}


class MeanStandardDeviationTradeoffMeasureParametricFunctionWrapper
: public FunctionImplementation
{
public:
  MeanStandardDeviationTradeoffMeasureParametricFunctionWrapper(const Point & x,
                                                                const Function & function,
                                                                const Distribution & distribution)
  : FunctionImplementation()
  , x_(x)
  , function_(function)
  , distribution_(distribution)
  {
    // Nothing to do
  }

  virtual MeanStandardDeviationTradeoffMeasureParametricFunctionWrapper * clone() const
  {
    return new MeanStandardDeviationTradeoffMeasureParametricFunctionWrapper(*this);
  }

  Point operator()(const Point & theta) const
  {
    Function function(function_);
    function.setParameter(theta);
    // (f_1(x), ...., f_d(x), f_1^2(x), ..., f_d^2(x))
    Point outP(function(x_));
    outP.add(outP);
    const UnsignedInteger outputDimension = function.getOutputDimension();
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
      outP[outputDimension + j] *= outP[j];
    return outP * distribution_.computePDF(theta);
  }

  Sample operator()(const Sample & theta) const
  {
    Function function(function_);
    const UnsignedInteger outputDimension = function.getOutputDimension();
    const UnsignedInteger size = theta.getSize();
    Sample outS(size, outputDimension);
    for (UnsignedInteger i = 0; i < size; ++i)
    {
      function.setParameter(theta[i]);
      outS[i] = function(x_);
    }

    outS.stack(outS);
    const Sample pdf(distribution_.computePDF(theta));
    for (UnsignedInteger i = 0; i < size; ++i)
      {
	for (UnsignedInteger j = 0; j < outputDimension; ++j)
	  outS(i, outputDimension + j) *= outS(i, j);
	outS[i] *= pdf(i, 0);
      }
    return outS;
  }

  UnsignedInteger getInputDimension() const
  {
    return function_.getParameterDimension();
  }

  UnsignedInteger getOutputDimension() const
  {
    return 2 * function_.getOutputDimension();
  }

  Description getInputDescription() const
  {
    return function_.getParameterDescription();
  }

  Description getOutputDescription() const
  {
    Description outputDescription(function_.getOutputDescription());
    outputDescription.add(outputDescription);
    return outputDescription;
  }

protected:
  Point x_;
  Function function_;
  Distribution distribution_;
};


/* Evaluation */
Point MeanStandardDeviationTradeoffMeasure::operator()(const Point & inP) const
{
  Function function(getFunction());
  const UnsignedInteger outputDimension = function.getOutputDimension();
  Point outP(outputDimension);
  if (getDistribution().isContinuous())
  {
    GaussKronrod gkr;
    gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("MeanStandardDeviationTradeoffMeasure-GaussKronrodRule")));
    const IteratedQuadrature algo(gkr);
    Pointer<FunctionImplementation> p_wrapper(new MeanStandardDeviationTradeoffMeasureParametricFunctionWrapper(inP, function, getDistribution()));
    const Function G(p_wrapper);
    // integrate (f_1(x), ...., f_d(x), f_1^2(x), ..., f_d^2(x))
    const Point integral(algo.integrate(G, getDistribution().getRange()));
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
    {
      const Scalar mean = integral[j];
      // Var(f(x))=\mathbb{E}(f^2(x))-\mathbb{E}(f(x))^2
      const Scalar variance = integral[outputDimension + j] - mean * mean;
      outP[j] = (1.0 - alpha_[j]) * mean + alpha_[j] * sqrt(variance);
    }
  }
  else
  {
    const Point weights(getDistribution().getProbabilities());
    const Sample parameters(getDistribution().getSupport());
    const UnsignedInteger size = parameters.getSize();
    Sample values(size, outputDimension);
    for (UnsignedInteger i = 0; i < size; ++i)
    {
      function.setParameter(parameters[i]);
      values[i] = function(inP);
    }
    // Here we use a UserDefined distribution because the algorithm
    // to compute a standard deviation is quite involved in the case
    // of nonuniform weights
    const UserDefined discrete(values, weights);
    const Point mean(discrete.getMean());
    const Point standardDeviation(discrete.getStandardDeviation());
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
      outP[j] = (1.0 - alpha_[j]) * mean[j] + alpha_[j] * standardDeviation[j];
  } // discrete
  return outP;
}


/* Alpha coefficient accessor */
void MeanStandardDeviationTradeoffMeasure::setAlpha(const Point & alpha)
{
  const UnsignedInteger dimension = alpha.getDimension();
  for(UnsignedInteger j = 0; j < dimension; ++ j)
  {
    if (!(alpha[j] >= 0.0) || !(alpha[j] <= 1.0))
      throw InvalidArgumentException(HERE) << "Alpha should be in [0, 1]";
  }
  alpha_ = alpha;
}

Point MeanStandardDeviationTradeoffMeasure::getAlpha() const
{
  return alpha_;
}


/* String converter */
String MeanStandardDeviationTradeoffMeasure::__repr__() const
{
  OSS oss;
  oss << "class=" << MeanStandardDeviationTradeoffMeasure::GetClassName()
      << " alpha=" << alpha_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void MeanStandardDeviationTradeoffMeasure::save(Advocate & adv) const
{
  MeasureEvaluationImplementation::save(adv);
  adv.saveAttribute("alpha_", alpha_);
}

/* Method load() reloads the object from the StorageManager */
void MeanStandardDeviationTradeoffMeasure::load(Advocate & adv)
{
  MeasureEvaluationImplementation::load(adv);
  adv.loadAttribute("alpha_", alpha_);
}


} /* namespace OTROBOPT */
