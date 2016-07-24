//                                               -*- C++ -*-
/**
 *  @brief Variance measure
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
#include "otrobopt/VarianceMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/IteratedQuadrature.hxx>
#include <openturns/UserDefined.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(VarianceMeasure);

static Factory<VarianceMeasure> Factory_VarianceMeasure;


/* Default constructor */
VarianceMeasure::VarianceMeasure()
  : MeasureEvaluationImplementation()
{
  // Nothing to do
}

/* Parameter constructor */
VarianceMeasure::VarianceMeasure (const NumericalMathFunction & function,
                                  const Distribution & distribution)
  : MeasureEvaluationImplementation(function, distribution)

{
  // Nothing to do
}

/* Virtual constructor method */
VarianceMeasure * VarianceMeasure::clone() const
{
  return new VarianceMeasure(*this);
}


class VarianceMeasureParametricFunctionWrapper : public NumericalMathFunctionImplementation
{
public:
  VarianceMeasureParametricFunctionWrapper (const NumericalPoint & x,
                                            const NumericalMathFunction & function,
                                            const Distribution & distribution)
  : NumericalMathFunctionImplementation()
  , x_(x)
  , function_(function)
  , distribution_(distribution)
  {
    // Nothing to do
  }

  virtual VarianceMeasureParametricFunctionWrapper * clone() const
  {
    return new VarianceMeasureParametricFunctionWrapper(*this);
  }

  NumericalPoint operator()(const NumericalPoint & theta) const
  {
    NumericalMathFunction function(function_);
    // (f_1(x), ...., f_d(x), f_1^2(x), ..., f_d^2(x))
    NumericalPoint outP(function(x_, theta));
    outP.add(outP);
    const UnsignedInteger outputDimension = outP.getDimension();
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
NumericalPoint VarianceMeasure::operator()(const NumericalPoint & inP) const
{
  NumericalMathFunction function(getFunction());
  NumericalPoint parameter(function.getParameter());
  const UnsignedInteger outputDimension = function.getOutputDimension();
  NumericalPoint outP(outputDimension);
  if (getDistribution().isContinuous())
  {
    GaussKronrod gkr;
    gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("VarianceMeasure-GaussKronrodRule")));
    const IteratedQuadrature algo(gkr);

    Pointer<NumericalMathFunctionImplementation> p_wrapper(new VarianceMeasureParametricFunctionWrapper(inP, function, getDistribution()));
    const NumericalMathFunction G(p_wrapper);
    NumericalPoint integral(algo.integrate(G, getDistribution().getRange()));
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
    {
      const NumericalScalar mean = integral[j];
      // Var(f(x))=\mathbb{E}(f^2(x))-\mathbb{E}(f(x))^2
      outP[j] = integral[outputDimension + j] - mean * mean;
    }
  }
  else
  {
    // To benefit from possible parallelization
    const NumericalSample values(function(inP, getDistribution().getSupport()));
    const NumericalPoint weights(getDistribution().getProbabilities());
    // Here we use a UserDefined distribution because the algorithm
    // to compute a centered moment is quite involved in the case of
    // nonuniform weights
    outP = UserDefined(values, weights).getCenteredMoment(2);
  } // discrete
  function.setParameter(parameter);
  return outP;
}

/* String converter */
String VarianceMeasure::__repr__() const
{
  OSS oss;
  oss << "class=" << VarianceMeasure::GetClassName();
  return oss;
}

/* Method save() stores the object through the StorageManager */
void VarianceMeasure::save(Advocate & adv) const
{
  MeasureEvaluationImplementation::save(adv);
}

/* Method load() reloads the object from the StorageManager */
void VarianceMeasure::load(Advocate & adv)
{
  MeasureEvaluationImplementation::load(adv);
}


} /* namespace OTROBOPT */
