//                                               -*- C++ -*-
/**
 *  @brief Tradeoff between mean and standard deviation
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
#include "otrobopt/MeanStandardDeviationTradeoffMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/IteratedQuadrature.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(MeanStandardDeviationTradeoffMeasure);

static Factory<MeanStandardDeviationTradeoffMeasure> RegisteredFactory;


/* Default constructor */
MeanStandardDeviationTradeoffMeasure::MeanStandardDeviationTradeoffMeasure()
  : MeasureEvaluationImplementation()
{
  // Nothing to do
}

/* Parameter constructor */
MeanStandardDeviationTradeoffMeasure::MeanStandardDeviationTradeoffMeasure (const Distribution & distribution,
                                                                            const NumericalMathFunction & function,
                                                                            const NumericalPoint & alpha)
  : MeasureEvaluationImplementation(distribution, function)
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
  {}

  virtual MeanStandardDeviationTradeoffMeasureParametricFunctionWrapper * clone() const
  {
    return new MeanStandardDeviationTradeoffMeasureParametricFunctionWrapper(*this);
  }

  NumericalPoint operator()(const NumericalPoint & theta) const
  {
    NumericalMathFunction function(function_);
    // (f(x), f(x)^2)
    NumericalPoint outP(function(x_, theta));
    outP.add(outP);
    for (UnsignedInteger j = 0; j < function.getOutputDimension(); ++ j)
    {
      outP[2 * j + 1] *= outP[2 * j + 1];
    }
    return outP * distribution_.computePDF(theta);
  }

  NumericalSample operator()(const NumericalSample & theta) const
  {
    const UnsignedInteger size = theta.getSize();
    NumericalSample outS(size, getOutputDimension());
    for (UnsignedInteger i = 0; i < size; ++ i)
    {
      outS[i] = operator()(theta[i]);
    }
    return outS;
  }

  UnsignedInteger getInputDimension() const
  {
    return function_.getInputDimension();
  }

  UnsignedInteger getOutputDimension() const
  {
    return 2 * function_.getOutputDimension();
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
    gkr.setRule(GaussKronrodRule::G1K3);
    IteratedQuadrature algo(gkr);
    Pointer<NumericalMathFunctionImplementation> p_wrapper(new MeanStandardDeviationTradeoffMeasureParametricFunctionWrapper(inP, function, getDistribution()));
    NumericalMathFunction G(p_wrapper);
    // integrate (f(x), f^2(x))
    NumericalPoint integral(algo.integrate(G, getDistribution().getRange()));
    // Var(f(x))=\mathbb{E}(f^2(x))-\mathbb{E}(f(x))^2
    NumericalScalar variance = integral[1] - integral[0] * integral[0];
    outP[0] = (1.0 - alpha_[0]) * integral[0] + alpha_[0] * sqrt(variance);
  }
  else
  {
    NumericalSample support(getDistribution().getSupport());
    const UnsignedInteger size = support.getSize();
    NumericalSample y(size, outputDimension);
    for (UnsignedInteger i = 0; i < size; ++ i)
    {
      y[i] = function(inP, support[i]);
    }
    outP = (1.0 - alpha_[0]) * y.computeMean() + alpha_[0] * y.computeStandardDeviationPerComponent();
  }
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
      throw InvalidArgumentException(HERE) << "Alpha should be in (0, 1)";
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
