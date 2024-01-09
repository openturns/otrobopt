//                                               -*- C++ -*-
/**
 *  @brief Tradeoff between mean and standard deviation
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
#include "otrobopt/MeanStandardDeviationTradeoffMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/IteratedQuadrature.hxx>
#include <openturns/UserDefined.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(MeanStandardDeviationTradeoffMeasure)

static Factory<MeanStandardDeviationTradeoffMeasure> Factory_MeanStandardDeviationTradeoffMeasure;


/* Default constructor */
MeanStandardDeviationTradeoffMeasure::MeanStandardDeviationTradeoffMeasure()
  : MeasureEvaluationImplementation()
{
  // Set the default integration algorithm
  GaussKronrod gkr;
  gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("MeanStandardDeviationTradeoffMeasure-GaussKronrodRule")));
  integrationAlgorithm_ = IteratedQuadrature(gkr);
}

/* Parameter constructor */
MeanStandardDeviationTradeoffMeasure::MeanStandardDeviationTradeoffMeasure (const Function & function,
    const Distribution & distribution,
    const Point & alpha)
  : MeasureEvaluationImplementation(function, distribution)
{
  setAlpha(alpha);
  // Set the default integration algorithm
  GaussKronrod gkr;
  gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("MeanStandardDeviationTradeoffMeasure-GaussKronrodRule")));
  integrationAlgorithm_ = IteratedQuadrature(gkr);
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
      const Distribution & distribution,
      const Scalar pdfThreshold)
    : FunctionImplementation()
    , x_(x)
    , function_(function)
    , distribution_(distribution)
    , pdfThreshold_(pdfThreshold)
  {
    // Nothing to do
  }

  virtual MeanStandardDeviationTradeoffMeasureParametricFunctionWrapper * clone() const
  {
    return new MeanStandardDeviationTradeoffMeasureParametricFunctionWrapper(*this);
  }

  Point operator()(const Point & theta) const
  {
    const Scalar pdf = distribution_.computePDF(theta);
    // (f_1(x), ...., f_d(x), f_1^2(x), ..., f_d^2(x))
    const UnsignedInteger outputDimension = function_.getOutputDimension();
    Point outP(2 * outputDimension, 0.0);
    if (pdf <= pdfThreshold_) return outP;
    Function function(function_);
    function.setParameter(theta);
    const Point outF(function(x_));
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
      {
        const Scalar fj = outF[j];
        outP[j] = fj * pdf;
        outP[outputDimension + j] = fj * outP[j];
      }
    return outP;
  }

  Sample operator()(const Sample & theta) const
  {
    const Point pdfs(distribution_.computePDF(theta).asPoint());
    Indices significant(0);
    const UnsignedInteger size = theta.getSize();
    for (UnsignedInteger i = 0; i < size; ++i)
      if (pdfs[i] > pdfThreshold_) significant.add(i);
    // (f_1(x), ...., f_d(x), f_1^2(x), ..., f_d^2(x))
    const UnsignedInteger outputDimension = function_.getOutputDimension();
    Sample outS(size, 2 * outputDimension);
    // Early exit to avoid the copy of function_
    if (significant.getSize() == 0) return outS;
    Function function(function_);
    for (UnsignedInteger i = 0; i < significant.getSize(); ++i)
    {
      const UnsignedInteger j = significant[i];
      function.setParameter(theta[j]);
      const Point fJ = function(x_);
      const Scalar pdfJ = pdfs[j];
      for (UnsignedInteger k = 0; k < outputDimension; ++k)
        {
          const Scalar fk = fJ[k];
          outS(j, k) = fk * pdfJ;
          outS(j, outputDimension + k) = fk * outS(j, k);
        } // k
    } // i
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
  Scalar pdfThreshold_;
};


/* Evaluation */
Point MeanStandardDeviationTradeoffMeasure::operator()(const Point & inP) const
{
  Function function(getFunction());
  const UnsignedInteger outputDimension = function.getOutputDimension();
  Point outP(outputDimension);
  if (getDistribution().isContinuous())
  {
    Pointer<FunctionImplementation> p_wrapper(new MeanStandardDeviationTradeoffMeasureParametricFunctionWrapper(inP, function, getDistribution(), pdfThreshold_));
    const Function G(p_wrapper);
    // integrate (f_1(x), ...., f_d(x), f_1^2(x), ..., f_d^2(x))
    const Point integral(integrationAlgorithm_.integrate(G, getDistribution().getRange()));
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
    const Point pdfs(getDistribution().getProbabilities());
    const Sample parameters(getDistribution().getSupport());
    const UnsignedInteger size = parameters.getSize();
    Sample values(0, outputDimension);
    Point weights(0);
    for (UnsignedInteger i = 0; i < size; ++i)
    {
      if (pdfs[i] > pdfThreshold_)
        {
          function.setParameter(parameters[i]);
          values.add(function(inP));
          weights.add(pdfs[i]);
        }
    } // for

    // Here we use a UserDefined distribution because the algorithm
    // to compute a central moment is quite involved in the case of
    // nonuniform weights
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
