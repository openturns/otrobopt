//                                               -*- C++ -*-
/**
 *  @brief Individual probability measure
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
IndividualChanceMeasure::IndividualChanceMeasure (const Function & function,
                                                  const Distribution & distribution,
                                                  const ComparisonOperator & op,
                                                  const Point & alpha)
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


class IndividualChanceMeasureParametricFunctionWrapper : public FunctionImplementation
{
public:
  IndividualChanceMeasureParametricFunctionWrapper (const Point & x,
                                                    const Function & function,
                                                    const Distribution & distribution)
  : FunctionImplementation()
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

  Point operator()(const Point & theta) const
  {
    Function function(function_);
    function.setParameter(theta);
    Point y(function(x_));
    for (UnsignedInteger j = 0; j < getOutputDimension(); ++ j)
      y[j] = (y[j] >= 0.0) ? 1.0 : 0.0;
    return y * distribution_.computePDF(theta);
  }

  Sample operator()(const Sample & theta) const
  {
    const UnsignedInteger size = theta.getSize();
    Sample outS(size, function_.getOutputDimension());
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
  Point x_;
  Function function_;
  Distribution distribution_;
};


/* Evaluation */
Point IndividualChanceMeasure::operator()(const Point & inP) const
{
  Function function(getFunction());
  const UnsignedInteger outputDimension = function.getOutputDimension();
  Point outP(outputDimension);
  if (getDistribution().isContinuous())
  {
    GaussKronrod gkr;
    gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("IndividualChanceMeasure-GaussKronrodRule")));
    const IteratedQuadrature algo(gkr);
    Pointer<FunctionImplementation> p_wrapper(new IndividualChanceMeasureParametricFunctionWrapper(inP, function, getDistribution()));
    const Function G(p_wrapper);
    outP = algo.integrate(G, getDistribution().getRange());
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
    // Here we compute the marginal complementary CDF locally to avoid
    // the creation cost of the marginal UserDefined distributions
    for (UnsignedInteger i = 0; i < size; ++ i)
    {
      for (UnsignedInteger j = 0; j < outputDimension; ++ j)
      {
        if (values(i, j) >= 0.0)
          outP[j] += weights[j];
      } // for j
    } // for i
  } // discrete
  return operator_.operator()(1.0, 2.0) ? alpha_ - outP : outP - alpha_;;
}

/* Alpha coefficient accessor */
void IndividualChanceMeasure::setAlpha(const Point & alpha)
{
  const UnsignedInteger dimension = alpha.getDimension();
  for(UnsignedInteger j = 0; j < dimension; ++ j)
  {
    if (!(alpha[j] >= 0.0) || !(alpha[j] <= 1.0))
      throw InvalidArgumentException(HERE) << "Alpha should be in (0, 1)";
  }
  alpha_ = alpha;
}

Point IndividualChanceMeasure::getAlpha() const
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
