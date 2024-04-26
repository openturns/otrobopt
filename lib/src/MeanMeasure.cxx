//                                               -*- C++ -*-
/**
 *  @brief Mean measure
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
#include "otrobopt/MeanMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/IteratedQuadrature.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(MeanMeasure)

static Factory<MeanMeasure> Factory_MeanMeasure;


/* Default constructor */
MeanMeasure::MeanMeasure()
  : MeasureEvaluationImplementation()
{
  // Set the default integration algorithm
  GaussKronrod gkr;
  gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("MeanMeasure-GaussKronrodRule")));
  integrationAlgorithm_ = IteratedQuadrature(gkr);
}

/* Parameter constructor */
MeanMeasure::MeanMeasure (const Function & function,
                          const Distribution & distribution)
  : MeasureEvaluationImplementation(function, distribution)
{
  // Set the default integration algorithm
  GaussKronrod gkr;
  gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("MeanMeasure-GaussKronrodRule")));
  integrationAlgorithm_ = IteratedQuadrature(gkr);
}

/* Virtual constructor method */
MeanMeasure * MeanMeasure::clone() const
{
  return new MeanMeasure(*this);
}


class MeanMeasureParametricFunctionWrapper : public FunctionImplementation
{
public:
  MeanMeasureParametricFunctionWrapper (const Point & x,
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

  virtual MeanMeasureParametricFunctionWrapper * clone() const
  {
    return new MeanMeasureParametricFunctionWrapper(*this);
  }

  Point operator()(const Point & theta) const
  {
    const Scalar pdf = distribution_.computePDF(theta);
    if (pdf <= pdfThreshold_) return Point(function_.getOutputDimension(), 0.0);
    Function function(function_);
    function.setParameter(theta);
    return function(x_) * pdf;
  }

  Sample operator()(const Sample & theta) const
  {
    const Point pdfs(distribution_.computePDF(theta).asPoint());
    Indices significant(0);
    const UnsignedInteger size = theta.getSize();
    const UnsignedInteger outputDimension = function_.getOutputDimension();
    Sample outS(size, outputDimension);
    for (UnsignedInteger i = 0; i < size; ++i)
      if (pdfs[i] > pdfThreshold_) significant.add(i);
    // Early exit to avoid the copy of function_
    if (significant.getSize() == 0) return outS;
    Function function(function_);
    for (UnsignedInteger i = 0; i < significant.getSize(); ++i)
    {
      const UnsignedInteger j = significant[i];
      function.setParameter(theta[j]);
      outS[j] = function(x_) * pdfs[j];
    } // for
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
  Scalar pdfThreshold_;
};


/* Evaluation */
Point MeanMeasure::operator()(const Point & inP) const
{
  Function function(getFunction());
  const UnsignedInteger outputDimension = function.getOutputDimension();
  Point outP(outputDimension);
  if (getDistribution().isContinuous())
  {
    Pointer<FunctionImplementation> p_wrapper(new MeanMeasureParametricFunctionWrapper(inP, function, getDistribution(), pdfThreshold_));
    const Function G(p_wrapper);
    outP = integrationAlgorithm_.integrate(G, getDistribution().getRange());
  }
  else
  {
    const Point pdfs(getDistribution().getProbabilities());
    const Sample parameters(getDistribution().getSupport());
    const UnsignedInteger size = parameters.getSize();
    for (UnsignedInteger i = 0; i < size; ++i)
    {
      if (pdfs[i] > pdfThreshold_)
      {
        function.setParameter(parameters[i]);
        outP += function(inP) * pdfs[i];
      }
    } // for
  } // !isContinuous
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
