//                                               -*- C++ -*-
/**
 *  @brief Mean measure
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
#include "otrobopt/MeanMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/IteratedQuadrature.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(MeanMeasure);

static Factory<MeanMeasure> Factory_MeanMeasure;


/* Default constructor */
MeanMeasure::MeanMeasure()
  : MeasureEvaluationImplementation()
{
  // Nothing to do
}

/* Parameter constructor */
MeanMeasure::MeanMeasure (const NumericalMathFunction & function,
                          const Distribution & distribution)
  : MeasureEvaluationImplementation(function, distribution)

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
  {
    // Nothing to do
  }

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
    NumericalMathFunction function(function_);
    NumericalSample outS(function(x_, theta));
    const NumericalSample pdfS(distribution_.computePDF(theta));
    const UnsignedInteger size = theta.getSize();
    for (UnsignedInteger i = 0; i < size; ++ i)
    {
      outS[i] *= pdfS[i][0];
    }
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
NumericalPoint MeanMeasure::operator()(const NumericalPoint & inP) const
{
  NumericalMathFunction function(getFunction());
  NumericalPoint parameter(function.getParameter());
  const UnsignedInteger outputDimension = function.getOutputDimension();
  NumericalPoint outP(outputDimension);
  if (getDistribution().isContinuous())
  {
    GaussKronrod gkr;
    gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("MeanMeasure-GaussKronrodRule")));
    const IteratedQuadrature algo(gkr);
    Pointer<NumericalMathFunctionImplementation> p_wrapper(new MeanMeasureParametricFunctionWrapper(inP, function, getDistribution()));
    const NumericalMathFunction G(p_wrapper);
    outP = algo.integrate(G, getDistribution().getRange());
  }
  else
  {
    // To benefit from possible parallelization
    const NumericalSample values(function(inP, getDistribution().getSupport()));
    const UnsignedInteger size = values.getSize();
    const NumericalPoint weights(getDistribution().getProbabilities());
    for (UnsignedInteger i = 0; i < size; ++ i)
      outP += values[i] * weights[i];
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
  MeasureEvaluationImplementation::save(adv);
}

/* Method load() reloads the object from the StorageManager */
void MeanMeasure::load(Advocate & adv)
{
  MeasureEvaluationImplementation::load(adv);
}


} /* namespace OTROBOPT */
