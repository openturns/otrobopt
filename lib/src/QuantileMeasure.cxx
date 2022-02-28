//                                               -*- C++ -*-
/**
 *  @brief Measure that evaluates as a quantile
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
#include "otrobopt/QuantileMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/IteratedQuadrature.hxx>
#include <openturns/Brent.hxx>
#include <openturns/UserDefined.hxx>


using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(QuantileMeasure)

static Factory<QuantileMeasure> Factory_QuantileMeasure;


/* Default constructor */
QuantileMeasure::QuantileMeasure()
  : MeasureEvaluationImplementation()
  , alpha_(0.0)
{
  // Set the default integration algorithm
  GaussKronrod gkr;
  gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("QuantileMeasure-GaussKronrodRule")));
  integrationAlgorithm_ = IteratedQuadrature(gkr);
}

/* Parameter constructor */
QuantileMeasure::QuantileMeasure (const Function & function,
                                  const Distribution & distribution,
                                  const Scalar alpha)
  : MeasureEvaluationImplementation(function, distribution)
{
  setAlpha(alpha);
  if (function.getOutputDimension() > 1) throw InvalidArgumentException(HERE) << "Quantile are only computed for 1-d functions.";
  setOutputDescription(Description(1, "P"));
  // Set the default integration algorithm
  GaussKronrod gkr;
  gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("QuantileMeasure-GaussKronrodRule")));
  integrationAlgorithm_ = IteratedQuadrature(gkr);
}

/* Virtual constructor method */
QuantileMeasure * QuantileMeasure::clone() const
{
  return new QuantileMeasure(*this);
}

/* This function is the kernel of the CDF computation of f(x,\Theta):
   for a given value of x \in R^d, a given value of \Theta \in R^p,
   a given value of s \in R, it returns 1_{f(x,\Theta)<=s} */
class QuantileMeasureParametricFunctionWrapper : public FunctionImplementation
{
public:
  QuantileMeasureParametricFunctionWrapper(const Point & x,
      const Function & function,
      const Distribution & distribution,
      const Scalar s)
    : FunctionImplementation()
    , x_(x)
    , function_(function)
    , distribution_(distribution)
    , s_(s)
  {}

  virtual QuantileMeasureParametricFunctionWrapper * clone() const
  {
    return new QuantileMeasureParametricFunctionWrapper(*this);
  }

  Point operator()(const Point & theta) const
  {
    Function function(function_);
    function.setParameter(theta);
    const Scalar y = function(x_)[0];
    const Scalar p = (y <= s_ ? distribution_.computePDF(theta) : 0.0);
    return Point(1, p);
  }

  Sample operator()(const Sample & theta) const
  {
    Function function(function_);
    const UnsignedInteger size = theta.getSize();
    Sample outS(size, 1);
    for (UnsignedInteger i = 0; i < size; ++ i)
    {
      function.setParameter(theta[i]);
      const Scalar y = function(x_)[0];
      outS(i, 0) = (y <= s_ ? distribution_.computePDF(theta[i]) : 0.0);
    }
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
  Point x_;
  Function function_;
  Distribution distribution_;
  Scalar s_;
};

class QuantileMeasureParametricFunctionWrapper2 : public FunctionImplementation
{
public:
  QuantileMeasureParametricFunctionWrapper2(const Point & x,
      const Function & function,
      const Distribution & distribution,
      const IntegrationAlgorithm & algorithm)
    : FunctionImplementation()
    , x_(x)
    , function_(function)
    , distribution_(distribution)
    , integrationAlgorithm_(algorithm)
  {
    // Nothing to do
  }

  virtual QuantileMeasureParametricFunctionWrapper2 * clone() const
  {
    return new QuantileMeasureParametricFunctionWrapper2(*this);
  }

  Point operator()(const Point & s) const
  {
    Pointer<FunctionImplementation> p_wrapper(new QuantileMeasureParametricFunctionWrapper(x_, function_, distribution_, s[0]));
    const Function G(p_wrapper);
    return integrationAlgorithm_.integrate(G, distribution_.getRange());
  }

  Sample operator()(const Sample & s) const
  {
    const UnsignedInteger size = s.getSize();
    Sample outS(size, getOutputDimension());
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
  Point x_;
  Function function_;
  Distribution distribution_;
  IntegrationAlgorithm integrationAlgorithm_;
};



/* Evaluation */
Point QuantileMeasure::operator()(const Point & inP) const
{
  Function function(getFunction());
  const UnsignedInteger outputDimension = function.getOutputDimension();
  Point outP(outputDimension);
  if (getDistribution().isContinuous())
  {
    Pointer<FunctionImplementation> p_wrapper(new QuantileMeasureParametricFunctionWrapper2(inP, function, getDistribution(), integrationAlgorithm_));
    Function G(p_wrapper);

    Scalar lower = 0.0;
    Scalar upper = 0.0;
    Scalar step = 1.0;
    Scalar cdfMin = G(Point(1, lower))[0];
    Scalar cdfMax = cdfMin;
    // Go backward until we find a point below the threshold
    while (cdfMin > alpha_)
    {
      upper = lower;
      cdfMax = cdfMin;
      lower -= step;
      cdfMin = G(Point(1, lower))[0];
      step *= 2.0;
    }
    // If the initial lower bound was an actual lower bound
    // Try to narrow the lower bound
    if (step == 1.0)
    {
      upper += step;
      cdfMax = G(Point(1, upper))[0];
      step *= 2.0;
      while (cdfMax <= alpha_)
      {
        lower = upper;
        cdfMin = cdfMax;
        upper += step;
        cdfMax = G(Point(1, upper))[0];
        step *= 2.0;
      }
    } // step == 1
    const Brent solver;
    outP[0] = solver.solve(G, alpha_, lower, upper, cdfMin, cdfMax);
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
    // to compute a quantile is quite involved in the case of nonuniform
    // weights
    outP = UserDefined(values, weights).computeQuantile(alpha_);
  }
  return outP;
}

/* Alpha coefficient accessor */
void QuantileMeasure::setAlpha(const Scalar alpha)
{
  if (!(alpha >= 0.0) || !(alpha <= 1.0))
    throw InvalidArgumentException(HERE) << "Alpha should be in (0, 1)";
  alpha_ = alpha;
}

Scalar QuantileMeasure::getAlpha() const
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
