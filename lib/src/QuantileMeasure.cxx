//                                               -*- C++ -*-
/**
 *  @brief Measure that evaluates as a quantile
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
#include "otrobopt/QuantileMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/IteratedQuadrature.hxx>
#include <openturns/Brent.hxx>
#include <openturns/UserDefined.hxx>


using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(QuantileMeasure);

static Factory<QuantileMeasure> Factory_QuantileMeasure;


/* Default constructor */
QuantileMeasure::QuantileMeasure()
  : MeasureEvaluationImplementation()
  , alpha_(0.0)
{
  // Nothing to do
}

/* Parameter constructor */
QuantileMeasure::QuantileMeasure (const NumericalMathFunction & function,
                                  const Distribution & distribution,
                                  const NumericalScalar alpha)
  : MeasureEvaluationImplementation(function, distribution)
{
  setAlpha(alpha);
  if (function.getOutputDimension() > 1) throw InvalidArgumentException(HERE) << "Quantile are only computed for 1-d functions.";
  setOutputDescription(Description(1, "P"));
}

/* Virtual constructor method */
QuantileMeasure * QuantileMeasure::clone() const
{
  return new QuantileMeasure(*this);
}

/* This function is the kernel of the CDF computation of f(x,\Theta):
   for a given value of x \in R^d, a given value of \Theta \in R^p,
   a given value of s \in R, it returns 1_{f(x,\Theta)<=s} */
class QuantileMeasureParametricFunctionWrapper : public NumericalMathFunctionImplementation
{
public:
  QuantileMeasureParametricFunctionWrapper(const NumericalPoint & x,
                                           const NumericalMathFunction & function,
                                           const Distribution & distribution,
                                           const NumericalScalar s)
  : NumericalMathFunctionImplementation()
  , x_(x)
  , function_(function)
  , distribution_(distribution)
  , s_(s)
  {}

  virtual QuantileMeasureParametricFunctionWrapper * clone() const
  {
    return new QuantileMeasureParametricFunctionWrapper(*this);
  }

  NumericalPoint operator()(const NumericalPoint & theta) const
  {
    NumericalMathFunction function(function_);
    const NumericalScalar y = function(x_, theta)[0];
    const NumericalScalar p = (y <= s_ ? distribution_.computePDF(theta) : 0.0);
    return NumericalPoint(1, p);
  }

  NumericalSample operator()(const NumericalSample & theta) const
  {
    const UnsignedInteger size = theta.getSize();
    NumericalSample outS(size, getOutputDimension());
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
    return 1;
  }

  Description getInputDescription() const
  {
    return function_.getParameterDescription();
  }

  Description getOutputDescription() const
  {
    return Description(1, "P");
  }

protected:
  NumericalPoint x_;
  NumericalMathFunction function_;
  Distribution distribution_;
  NumericalScalar s_;
};

class QuantileMeasureParametricFunctionWrapper2 : public NumericalMathFunctionImplementation
{
public:
  QuantileMeasureParametricFunctionWrapper2(const NumericalPoint & x,
                                            const NumericalMathFunction & function,
                                            const Distribution & distribution)
  : NumericalMathFunctionImplementation()
  , x_(x)
  , function_(function)
  , distribution_(distribution)
  {
    // Nothing to do
  }

  virtual QuantileMeasureParametricFunctionWrapper2 * clone() const
  {
    return new QuantileMeasureParametricFunctionWrapper2(*this);
  }

  NumericalPoint operator()(const NumericalPoint & s) const
  {
    GaussKronrod gkr;
    gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("QuantileMeasure-GaussKronrodRule")));
    const IteratedQuadrature algo(gkr);
    Pointer<NumericalMathFunctionImplementation> p_wrapper(new QuantileMeasureParametricFunctionWrapper(x_, function_, distribution_, s[0]));
    const NumericalMathFunction G(p_wrapper);
    return algo.integrate(G, distribution_.getRange());
  }

  NumericalSample operator()(const NumericalSample & s) const
  {
    const UnsignedInteger size = s.getSize();
    NumericalSample outS(size, getOutputDimension());
    for (UnsignedInteger i = 0; i < size; ++ i)
      outS[i] = operator()(s[i]);
    return outS;
  }

  UnsignedInteger getInputDimension() const
  {
    return 1;
  }

  UnsignedInteger getOutputDimension() const
  {
    return 1;
  }

  Description getInputDescription() const
  {
    return Description(1, "s");
  }

  Description getOutputDescription() const
  {
    return Description(1, "P");
  }

protected:
  NumericalPoint x_;
  NumericalMathFunction function_;
  Distribution distribution_;
};



/* Evaluation */
NumericalPoint QuantileMeasure::operator()(const NumericalPoint & inP) const
{
  NumericalMathFunction function(getFunction());
  NumericalPoint parameter(function.getParameter());
  const UnsignedInteger outputDimension = function.getOutputDimension();
  NumericalPoint outP(outputDimension);
  if (getDistribution().isContinuous())
  {
    Pointer<NumericalMathFunctionImplementation> p_wrapper(new QuantileMeasureParametricFunctionWrapper2(inP, function, getDistribution()));
    NumericalMathFunction G(p_wrapper);

    NumericalScalar lower = 0.0;
    NumericalScalar upper = 0.0;
    NumericalScalar step = 1.0;
    NumericalScalar cdfMin = G(NumericalPoint(1, lower))[0];
    NumericalScalar cdfMax = cdfMin;
    // Go backward until we find a point below the threshold
    while (cdfMin > alpha_)
    {
      upper = lower;
      cdfMax = cdfMin;
      lower -= step;
      cdfMin = G(NumericalPoint(1, lower))[0];
      step *= 2.0;
    }
    // If the initial lower bound was an actual lower bound
    // Try to narrow the lower bound
    if (step == 1.0)
    {
      upper += step;
      cdfMax = G(NumericalPoint(1, upper))[0];
      step *= 2.0;
      while (cdfMax <= alpha_)
      {
        lower = upper;
        cdfMin = cdfMax;
        upper += step;
        cdfMax = G(NumericalPoint(1, upper))[0];
        step *= 2.0;
      }
    } // step == 1
    const Brent solver;
    outP[0] = solver.solve(G, alpha_, lower, upper, cdfMin, cdfMax);
  }
  else
  {
    // To benefit from possible parallelization
    const NumericalSample values(function(inP, getDistribution().getSupport()));
    const NumericalPoint weights(getDistribution().getProbabilities());
    // Here we use a UserDefined distribution because the algorithm
    // to compute a quantile is quite involved in the case of nonuniform
    // weights
    outP = UserDefined(values, weights).computeQuantile(alpha_);
  }
  function.setParameter(parameter);
  return outP;
}

/* Alpha coefficient accessor */
void QuantileMeasure::setAlpha(const NumericalScalar alpha)
{
  if (!(alpha >= 0.0) || !(alpha <= 1.0))
    throw InvalidArgumentException(HERE) << "Alpha should be in (0, 1)";
  alpha_ = alpha;
}

NumericalScalar QuantileMeasure::getAlpha() const
{
  return alpha_;
}

/* String converter */
String QuantileMeasure::__repr__() const
{
  OSS oss;
  oss << "class=" << QuantileMeasure::GetClassName()
      << " alpha=" << alpha_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void QuantileMeasure::save(Advocate & adv) const
{
  MeasureEvaluationImplementation::save(adv);
  adv.saveAttribute("alpha_", alpha_);
}

/* Method load() reloads the object from the StorageManager */
void QuantileMeasure::load(Advocate & adv)
{
  MeasureEvaluationImplementation::load(adv);
  adv.loadAttribute("alpha_", alpha_);
}


} /* namespace OTROBOPT */
