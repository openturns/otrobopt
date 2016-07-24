//                                               -*- C++ -*-
/**
 *  @brief Tradeoff between mean and standard deviation
 *
 *  Copyright 2005-2016 Airbus-EDF-IMACS-Phimeca
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
MeanStandardDeviationTradeoffMeasure::MeanStandardDeviationTradeoffMeasure (const NumericalMathFunction & function,
                                                                            const Distribution & distribution,
                                                                            const NumericalPoint & alpha)
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
: public NumericalMathFunctionImplementation
{
public:
  MeanStandardDeviationTradeoffMeasureParametricFunctionWrapper(const NumericalPoint & x,
                                                                const NumericalMathFunction & function,
                                                                const Distribution & distribution)
  : NumericalMathFunctionImplementation()
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

  NumericalPoint operator()(const NumericalPoint & theta) const
  {
    NumericalMathFunction function(function_);
    // (f_1(x), ...., f_d(x), f_1^2(x), ..., f_d^2(x))
    NumericalPoint outP(function(x_, theta));
    outP.add(outP);
    const UnsignedInteger outputDimension = function_.getOutputDimension();
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
      outP[outputDimension + j] *= outP[j];
    return outP * distribution_.computePDF(theta);
  }

  NumericalSample operator()(const NumericalSample & theta) const
  {
    // To benefit from possible parallelization
    NumericalMathFunction function(function_);
    NumericalSample outS(function(x_, theta));
    const UnsignedInteger size = outS.getSize();
    const UnsignedInteger outputDimension = outS.getDimension();
    outS.stack(outS);
    const NumericalSample pdf(distribution_.computePDF(theta));
    for (UnsignedInteger i = 0; i < size; ++i)
      {
	for (UnsignedInteger j = 0; j < outputDimension; ++j)
	  outS[i][outputDimension + j] *= outS[i][j];
	outS[i] *= pdf[i][0];
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
  NumericalPoint x_;
  NumericalMathFunction function_;
  Distribution distribution_;
};


/* Evaluation */
NumericalPoint MeanStandardDeviationTradeoffMeasure::operator()(const NumericalPoint & inP) const
{
  NumericalMathFunction function(getFunction());
  NumericalPoint parameter(function.getParameter());
  const UnsignedInteger outputDimension = function.getOutputDimension();
  NumericalPoint outP(outputDimension);
  if (getDistribution().isContinuous())
  {
    GaussKronrod gkr;
    gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("MeanStandardDeviationTradeoffMeasure-GaussKronrodRule")));
    const IteratedQuadrature algo(gkr);
    Pointer<NumericalMathFunctionImplementation> p_wrapper(new MeanStandardDeviationTradeoffMeasureParametricFunctionWrapper(inP, function, getDistribution()));
    const NumericalMathFunction G(p_wrapper);
    // integrate (f_1(x), ...., f_d(x), f_1^2(x), ..., f_d^2(x))
    const NumericalPoint integral(algo.integrate(G, getDistribution().getRange()));
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
    {
      const NumericalScalar mean = integral[j];
      // Var(f(x))=\mathbb{E}(f^2(x))-\mathbb{E}(f(x))^2
      const NumericalScalar variance = integral[outputDimension + j] - mean * mean;
      outP[j] = (1.0 - alpha_[j]) * mean + alpha_[j] * sqrt(variance);
    }
  }
  else
  {
    // To benefit from possible parallelization
    const NumericalSample values(function(inP, getDistribution().getSupport()));
    const NumericalPoint weights(getDistribution().getProbabilities());
    // Here we use a UserDefined distribution because the algorithm
    // to compute a standard deviation is quite involved in the case
    // of nonuniform weights
    const UserDefined discrete(values, weights);
    const NumericalPoint mean(discrete.getMean());
    const NumericalPoint standardDeviation(discrete.getStandardDeviation());
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
      outP[j] = (1.0 - alpha_[j]) * mean[j] + alpha_[j] * standardDeviation[j];
  } // discrete
  function.setParameter(parameter);
  return outP;
}


/* Alpha coefficient accessor */
void MeanStandardDeviationTradeoffMeasure::setAlpha(const NumericalPoint & alpha)
{
  const UnsignedInteger dimension = alpha.getDimension();
  for(UnsignedInteger j = 0; j < dimension; ++ j)
  {
    if (!(alpha[j] >= 0.0) || !(alpha[j] <= 1.0))
      throw InvalidArgumentException(HERE) << "Alpha should be in [0, 1]";
  }
  alpha_ = alpha;
}

NumericalPoint MeanStandardDeviationTradeoffMeasure::getAlpha() const
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
