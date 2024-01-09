//                                               -*- C++ -*-
/**
 *  @brief Variance measure
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
#include "otrobopt/VarianceMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/IteratedQuadrature.hxx>
#include <openturns/UserDefined.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(VarianceMeasure)

static Factory<VarianceMeasure> Factory_VarianceMeasure;


/* Default constructor */
VarianceMeasure::VarianceMeasure()
  : MeasureEvaluationImplementation()
{
  // Set the default integration algorithm
  GaussKronrod gkr;
  gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("VarianceMeasure-GaussKronrodRule")));
  integrationAlgorithm_ = IteratedQuadrature(gkr);
}

/* Parameter constructor */
VarianceMeasure::VarianceMeasure (const Function & function,
                                  const Distribution & distribution)
  : MeasureEvaluationImplementation(function, distribution)

{
  // Set the default integration algorithm
  GaussKronrod gkr;
  gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("VarianceMeasure-GaussKronrodRule")));
  integrationAlgorithm_ = IteratedQuadrature(gkr);
}

/* Virtual constructor method */
VarianceMeasure * VarianceMeasure::clone() const
{
  return new VarianceMeasure(*this);
}


class VarianceMeasureParametricFunctionWrapper : public FunctionImplementation
{
public:
  VarianceMeasureParametricFunctionWrapper (const Point & x,
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

  virtual VarianceMeasureParametricFunctionWrapper * clone() const
  {
    return new VarianceMeasureParametricFunctionWrapper(*this);
  }

  Point operator()(const Point & theta) const
  {
    const Scalar pdf = distribution_.computePDF(theta);
    const UnsignedInteger outputDimension = function_.getOutputDimension();
    // (f_1(x), ...., f_d(x), f_1^2(x), ..., f_d^2(x))
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
Point VarianceMeasure::operator()(const Point & inP) const
{
  Function function(getFunction());
  const UnsignedInteger outputDimension = function.getOutputDimension();
  Point outP(outputDimension);
  if (getDistribution().isContinuous())
  {
    Pointer<FunctionImplementation> p_wrapper(new VarianceMeasureParametricFunctionWrapper(inP, function, getDistribution(), pdfThreshold_));
    const Function G(p_wrapper);
    Point integral(integrationAlgorithm_.integrate(G, getDistribution().getRange()));
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
    {
      const Scalar mean = integral[j];
      // Var(f(x))=\mathbb{E}(f^2(x))-\mathbb{E}(f(x))^2
      outP[j] = integral[outputDimension + j] - mean * mean;
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
    outP = UserDefined(values, weights).getCentralMoment(2);
  } // discrete
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
