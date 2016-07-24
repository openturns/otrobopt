//                                               -*- C++ -*-
/**
 *  @brief Individual probability measure
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
#include "otrobopt/IndividualChanceMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/IteratedQuadrature.hxx>
#include <openturns/UserDefined.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(IndividualChanceMeasure);

static Factory<IndividualChanceMeasure> Factory_IndividualChanceMeasure;


/* Default constructor */
IndividualChanceMeasure::IndividualChanceMeasure()
  : MeasureEvaluationImplementation()
{
  // Nothing to do
}

/* Parameter constructor */
IndividualChanceMeasure::IndividualChanceMeasure (const NumericalMathFunction & function,
                                                  const Distribution & distribution,
                                                  const ComparisonOperator & op,
                                                  const NumericalPoint & alpha)
  : MeasureEvaluationImplementation(function, distribution)
  , operator_(op)
{
  setAlpha(alpha);
}

/* Virtual constructor method */
IndividualChanceMeasure * IndividualChanceMeasure::clone() const
{
  return new IndividualChanceMeasure(*this);
}


class IndividualChanceMeasureParametricFunctionWrapper : public NumericalMathFunctionImplementation
{
public:
  IndividualChanceMeasureParametricFunctionWrapper (const NumericalPoint & x,
                                                    const NumericalMathFunction & function,
                                                    const Distribution & distribution)
  : NumericalMathFunctionImplementation()
  , x_(x)
  , function_(function)
  , distribution_(distribution)
  {
    // Nothing to do
  }

  virtual IndividualChanceMeasureParametricFunctionWrapper * clone() const
  {
    return new IndividualChanceMeasureParametricFunctionWrapper(*this);
  }

  NumericalPoint operator()(const NumericalPoint & theta) const
  {
    NumericalMathFunction function(function_);
    NumericalPoint y(function(x_, theta));
    for (UnsignedInteger j = 0; j < getOutputDimension(); ++ j)
      y[j] = (y[j] >= 0.0) ? 1.0 : 0.0;
    return y * distribution_.computePDF(theta);
  }

  NumericalSample operator()(const NumericalSample & theta) const
  {
    const UnsignedInteger size = theta.getSize();
    NumericalSample outS(size, function_.getOutputDimension());
    for (UnsignedInteger i = 0; i < size; ++ i)
      outS[i] = operator()(theta[i]);
    return outS;
  }

  UnsignedInteger getInputDimension() const
  {
    return function_.getParameterDimension();
  }

  UnsignedInteger getOutputDimension() const
  {
    return function_.getOutputDimension();
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
  NumericalPoint x_;
  NumericalMathFunction function_;
  Distribution distribution_;
};


/* Evaluation */
NumericalPoint IndividualChanceMeasure::operator()(const NumericalPoint & inP) const
{
  NumericalMathFunction function(getFunction());
  NumericalPoint parameter(function.getParameter());
  const UnsignedInteger outputDimension = function.getOutputDimension();
  NumericalPoint outP(outputDimension);
  if (getDistribution().isContinuous())
  {
    GaussKronrod gkr;
    gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("IndividualChanceMeasure-GaussKronrodRule")));
    const IteratedQuadrature algo(gkr);
    Pointer<NumericalMathFunctionImplementation> p_wrapper(new IndividualChanceMeasureParametricFunctionWrapper(inP, function, getDistribution()));
    const NumericalMathFunction G(p_wrapper);
    outP = algo.integrate(G, getDistribution().getRange());
  }
  else
  {
    // To benefit from possible parallelization
    const NumericalSample values(function(inP, getDistribution().getSupport()));
    const NumericalPoint weights(getDistribution().getProbabilities());
    const UnsignedInteger size = values.getSize();
    // Here we compute the marginal complementary CDF locally to avoid
    // the creation cost of the marginal UserDefined distributions
    for (UnsignedInteger i = 0; i < size; ++ i)
    {
      for (UnsignedInteger j = 0; j < outputDimension; ++ j)
      {
        if (values[i][j] >= 0.0)
          outP[j] += weights[j];
      } // for j
    } // for i
  } // discrete
  function.setParameter(parameter);
  return operator_.operator()(1.0, 2.0) ? alpha_ - outP : outP - alpha_;;
}

/* Alpha coefficient accessor */
void IndividualChanceMeasure::setAlpha(const NumericalPoint & alpha)
{
  const UnsignedInteger dimension = alpha.getDimension();
  for(UnsignedInteger j = 0; j < dimension; ++ j)
  {
    if (!(alpha[j] >= 0.0) || !(alpha[j] <= 1.0))
      throw InvalidArgumentException(HERE) << "Alpha should be in (0, 1)";
  }
  alpha_ = alpha;
}

NumericalPoint IndividualChanceMeasure::getAlpha() const
{
  return alpha_;
}

/* String converter */
String IndividualChanceMeasure::__repr__() const
{
  OSS oss;
  oss << "class=" << IndividualChanceMeasure::GetClassName()
      << " alpha=" << alpha_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void IndividualChanceMeasure::save(Advocate & adv) const
{
  MeasureEvaluationImplementation::save(adv);
  adv.saveAttribute("alpha_", alpha_);
  adv.saveAttribute("operator_", operator_);
}

/* Method load() reloads the object from the StorageManager */
void IndividualChanceMeasure::load(Advocate & adv)
{
  MeasureEvaluationImplementation::load(adv);
  adv.loadAttribute("alpha_", alpha_);
  adv.loadAttribute("operator_", operator_);
}


} /* namespace OTROBOPT */
