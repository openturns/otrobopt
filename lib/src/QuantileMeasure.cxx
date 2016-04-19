//                                               -*- C++ -*-
/**
 *  @brief Measure that evaluates as a quantile
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
#include "otrobopt/QuantileMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/IteratedQuadrature.hxx>
#include <openturns/Brent.hxx>


using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(QuantileMeasure);

static Factory<QuantileMeasure> RegisteredFactory;


/* Default constructor */
QuantileMeasure::QuantileMeasure()
  : MeasureFunctionImplementation()
  , alpha_(0.0)
{
  // Nothing to do
}

/* Parameter constructor */
QuantileMeasure::QuantileMeasure (const Distribution & distribution,
                                  const NumericalMathFunction & function,
                                  const NumericalScalar alpha)
  : MeasureFunctionImplementation(distribution, function)
{
  setAlpha(alpha);
}

/* Virtual constructor method */
QuantileMeasure * QuantileMeasure::clone() const
{
  return new QuantileMeasure(*this);
}


class QuantileMeasureParametricFunctionWrapper : public NumericalMathFunctionImplementation
{
public:
  QuantileMeasureParametricFunctionWrapper(const NumericalPoint & x,
                                           const NumericalMathFunction & function,
                                           const NumericalScalar s)
  : NumericalMathFunctionImplementation()
  , x_(x)
  , function_(function)
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
    const NumericalScalar p = y < s_ ? 1.0 : 0.0;
    return NumericalPoint(1, p);
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
    return 1;
  }

protected:
  NumericalPoint x_;
  NumericalMathFunction function_;
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
  {}

  virtual QuantileMeasureParametricFunctionWrapper2 * clone() const
  {
    return new QuantileMeasureParametricFunctionWrapper2(*this);
  }

  NumericalPoint operator()(const NumericalPoint & s) const
  {
    GaussKronrod gkr;
    gkr.setRule(GaussKronrodRule::G1K3);
    const IteratedQuadrature algo(gkr);
    Pointer<NumericalMathFunctionImplementation> p_wrapper(new QuantileMeasureParametricFunctionWrapper(x_, function_, s[0]));
    const NumericalMathFunction G(p_wrapper);
    return algo.integrate(G, distribution_.getRange());
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
    return 1;
  }

  UnsignedInteger getOutputDimension() const
  {
    return 1;
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
  NumericalPoint outP(1);
  if (getDistribution().isContinuous())
  {
    Pointer<NumericalMathFunctionImplementation> p_wrapper(new QuantileMeasureParametricFunctionWrapper2(inP, function, getDistribution()));
    NumericalMathFunction G(p_wrapper);
    Brent solver;
    const NumericalScalar a = function(inP, getDistribution().getRange().getLowerBound())[0];
    const NumericalScalar b = function(inP, getDistribution().getRange().getUpperBound())[0];
    const NumericalScalar fA = 0.0;
    const NumericalScalar fB = 1.0;
    outP[0] = solver.solve(G, alpha_, a, b, fA, fB);
  }
  else
  {
    NumericalSample support(getDistribution().getSupport());
    const UnsignedInteger size = support.getSize();
    NumericalSample outS(size, function.getOutputDimension());
    for (UnsignedInteger i = 0; i < size; ++ i)
    {
      outS[i] = function(inP, support[i]);
    }
    outP[0] = outS.computeQuantile(alpha_)[0];
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
  MeasureFunctionImplementation::save(adv);
  adv.saveAttribute("alpha_", alpha_);
}

/* Method load() reloads the object from the StorageManager */
void QuantileMeasure::load(Advocate & adv)
{
  MeasureFunctionImplementation::load(adv);
  adv.loadAttribute("alpha_", alpha_);
}


} /* namespace OTROBOPT */
