//                                               -*- C++ -*-
/**
 *  @brief MeanMeasure
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
#include "otrobopt/MeanMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/IteratedQuadrature.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(MeanMeasure);

static Factory<MeanMeasure> RegisteredFactory;


/* Default constructor */
MeanMeasure::MeanMeasure()
  : MeasureFunctionImplementation()
{
  // Nothing to do
}

/* Parameter constructor */
MeanMeasure::MeanMeasure (const Distribution & distribution,
                          const NumericalMathFunction & function)
  : MeasureFunctionImplementation(distribution, function)

{
  // Nothing to do
}

/* Virtual constructor method */
MeanMeasure * MeanMeasure::clone() const
{
  return new MeanMeasure(*this);
}


class MeanMeasureParametricFunctionWrapper : public NumericalMathFunctionImplementation
{
public:
  MeanMeasureParametricFunctionWrapper (const NumericalPoint & x,
                                        const NumericalMathFunction & function,
                                        const Distribution & distribution)
  : NumericalMathFunctionImplementation()
  , x_(x)
  , function_(function)
  , distribution_(distribution)
  {}

  virtual MeanMeasureParametricFunctionWrapper * clone() const
  {
    return new MeanMeasureParametricFunctionWrapper(*this);
  }

  NumericalPoint operator()(const NumericalPoint & theta) const
  {
    NumericalMathFunction function(function_);
    return function(x_, theta) * distribution_.computePDF(theta);
  }

  NumericalSample operator()(const NumericalSample & theta) const
  {
    const UnsignedInteger size = theta.getSize();
    NumericalSample outS(size, function_.getOutputDimension());
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
    return function_.getOutputDimension();
  }

protected:
  NumericalPoint x_;
  NumericalMathFunction function_;
  Distribution distribution_;
};


/* Evaluation */
NumericalPoint MeanMeasure::operator()(const NumericalPoint & inP) const
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
    Pointer<NumericalMathFunctionImplementation> p_wrapper(new MeanMeasureParametricFunctionWrapper(inP, function, getDistribution()));
    NumericalMathFunction G(p_wrapper);
    outP = algo.integrate(G, getDistribution().getRange());
  }
  else
  {
    NumericalSample support(getDistribution().getSupport());
    const UnsignedInteger size = support.getSize();
    for (UnsignedInteger i = 0; i < size; ++ i)
    {
      outP += 1.0 / size * function(inP, support[i]);
    }
  }
  function.setParameter(parameter);
  return outP;
}

/* String converter */
String MeanMeasure::__repr__() const
{
  OSS oss;
  oss << "class=" << MeanMeasure::GetClassName();
  return oss;
}

/* Method save() stores the object through the StorageManager */
void MeanMeasure::save(Advocate & adv) const
{
  MeasureFunctionImplementation::save(adv);
}

/* Method load() reloads the object from the StorageManager */
void MeanMeasure::load(Advocate & adv)
{
  MeasureFunctionImplementation::load(adv);
}


} /* namespace OTROBOPT */
