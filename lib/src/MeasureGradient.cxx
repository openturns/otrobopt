//                                               -*- C++ -*-
/**
 *  @brief Measure gradient implementation
 *
 *  Copyright 2005-2026 Airbus-EDF-IMACS-ONERA-Phimeca
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
#include "otrobopt/MeasureGradient.hxx"
#include "otrobopt/MeasureEvaluationImplementation.hxx"

#include <openturns/Distribution.hxx>
#include <openturns/FunctionImplementation.hxx>
#include <openturns/IntegrationAlgorithm.hxx>
#include <openturns/PersistentObjectFactory.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(MeasureGradient)

static Factory<MeasureGradient> Factory_MeasureGradient;


/** Integrand of the measure gradient: the derivative of the integrand is
    flattened into a point and multiplied by the probability density. */
class MeasureGradientIntegrand
  : public FunctionImplementation
{
public:
  MeasureGradientIntegrand(const Point & x,
                           const MeasureEvaluation & evaluation,
                           const Point & auxiliaryValues)
    : FunctionImplementation()
    , x_(x)
    , evaluation_(evaluation)
    , auxiliaryValues_(auxiliaryValues)
  {
    // Nothing to do
  }

  MeasureGradientIntegrand * clone() const override
  {
    return new MeasureGradientIntegrand(*this);
  }

  Point operator()(const Point & theta) const override
  {
    const MeasureEvaluationImplementation * p_implementation = evaluation_.getImplementation().get();
    const UnsignedInteger inputDimension = p_implementation->getInputDimension();
    const UnsignedInteger outputDimension = p_implementation->getOutputDimension();
    Point outP(inputDimension * outputDimension, 0.0);
    const Scalar pdf = p_implementation->getDistribution().computePDF(theta);
    if (pdf <= p_implementation->getPDFThreshold()) return outP;
    const Matrix localGradient(p_implementation->integrandGradient(x_, theta, auxiliaryValues_));
    for (UnsignedInteger i = 0; i < inputDimension; ++i)
      for (UnsignedInteger j = 0; j < outputDimension; ++j)
        outP[i * outputDimension + j] = localGradient(i, j) * pdf;
    return outP;
  }

  Sample operator()(const Sample & theta) const override
  {
    const UnsignedInteger size = theta.getSize();
    Sample outS(size, getOutputDimension());
    for (UnsignedInteger i = 0; i < size; ++i)
      outS[i] = operator()(theta[i]);
    return outS;
  }

  UnsignedInteger getInputDimension() const override
  {
    return evaluation_.getFunction().getParameterDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    const MeasureEvaluationImplementation * p_implementation = evaluation_.getImplementation().get();
    return p_implementation->getInputDimension() * p_implementation->getOutputDimension();
  }

private:
  Point x_;
  MeasureEvaluation evaluation_;
  Point auxiliaryValues_;
};


/* Default constructor */
MeasureGradient::MeasureGradient()
  : GradientImplementation()
{
  // Nothing to do
}

/* Constructor from a measure evaluation */
MeasureGradient::MeasureGradient(const MeasureEvaluation & evaluation)
  : GradientImplementation()
  , evaluation_(evaluation)
{
  // Nothing to do
}

/* Constructor from the component measures of an aggregate */
MeasureGradient::MeasureGradient(const MeasureEvaluationCollection & collection)
  : GradientImplementation()
  , collection_(collection)
{
  buildComponentGradients();
}

/* Rebuild the component gradients of an aggregate */
void MeasureGradient::buildComponentGradients()
{
  const UnsignedInteger size = collection_.getSize();
  componentGradients_ = Collection<Gradient>(size);
  for (UnsignedInteger i = 0; i < size; ++i)
    componentGradients_[i] = collection_[i].getImplementation()->buildGradient();
}

/* Virtual constructor method */
MeasureGradient * MeasureGradient::clone() const
{
  return new MeasureGradient(*this);
}

/* Gradient */
Matrix MeasureGradient::gradient(const Point & inP) const
{
  const UnsignedInteger size = componentGradients_.getSize();
  if (size > 0)
  {
    // Aggregate mode: concatenate the component blocks column-wise
    Matrix outG(getInputDimension(), getOutputDimension());
    UnsignedInteger columnIndex = 0;
    for (UnsignedInteger i = 0; i < size; ++i)
    {
      const Matrix localGradient(componentGradients_[i].gradient(inP));
      const UnsignedInteger rowDimension = localGradient.getNbRows();
      const UnsignedInteger columnDimension = localGradient.getNbColumns();
      for (UnsignedInteger j = 0; j < rowDimension; ++j)
        for (UnsignedInteger k = 0; k < columnDimension; ++k)
          outG(j, columnIndex + k) = localGradient(j, k);
      columnIndex += columnDimension;
    }
    return outG;
  }
  // Integrand mode: integrate the derivative of the integrand
  const MeasureEvaluationImplementation * p_implementation = evaluation_.getImplementation().get();
  const Function function(p_implementation->getFunction());
  const UnsignedInteger inputDimension = function.getInputDimension();
  const UnsignedInteger outputDimension = function.getOutputDimension();
  const Point auxiliaryValues(p_implementation->computeAuxiliaryValues(inP));
  Matrix outG(inputDimension, outputDimension);
  const Distribution distribution(p_implementation->getDistribution());
  if (distribution.isContinuous())
  {
    Pointer<FunctionImplementation> p_integrand(new MeasureGradientIntegrand(inP, evaluation_, auxiliaryValues));
    const Function integrand(p_integrand);
    const Point integral(p_implementation->getIntegrationAlgorithm().integrate(integrand, distribution.getRange()));
    for (UnsignedInteger i = 0; i < inputDimension; ++i)
      for (UnsignedInteger j = 0; j < outputDimension; ++j)
        outG(i, j) = integral[i * outputDimension + j];
  }
  else
  {
    const Point pdfs(distribution.getProbabilities());
    const Sample parameters(distribution.getSupport());
    const UnsignedInteger parameterDimension = parameters.getSize();
    for (UnsignedInteger k = 0; k < parameterDimension; ++k)
    {
      if (pdfs[k] > p_implementation->getPDFThreshold())
      {
        const Matrix localGradient(p_implementation->integrandGradient(inP, parameters[k], auxiliaryValues));
        for (UnsignedInteger i = 0; i < inputDimension; ++i)
          for (UnsignedInteger j = 0; j < outputDimension; ++j)
            outG(i, j) += pdfs[k] * localGradient(i, j);
      }
    }
  }
  return outG;
}

/* Dimension accessors */
UnsignedInteger MeasureGradient::getInputDimension() const
{
  if (collection_.getSize() > 0)
    return collection_[0].getFunction().getInputDimension();
  return evaluation_.getImplementation()->getInputDimension();
}

UnsignedInteger MeasureGradient::getOutputDimension() const
{
  if (collection_.getSize() > 0)
  {
    UnsignedInteger outputDimension = 0;
    for (UnsignedInteger i = 0; i < collection_.getSize(); ++i)
      outputDimension += collection_[i].getOutputDimension();
    return outputDimension;
  }
  return evaluation_.getImplementation()->getOutputDimension();
}

/* String converter */
String MeasureGradient::__repr__() const
{
  OSS oss;
  oss << "class=" << MeasureGradient::GetClassName();
  if (collection_.getSize() > 0)
    oss << " collection=" << collection_;
  else
    oss << " evaluation=" << evaluation_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void MeasureGradient::save(Advocate & adv) const
{
  GradientImplementation::save(adv);
  adv.saveAttribute("evaluation_", evaluation_);
  adv.saveAttribute("collection_", collection_);
}

/* Method load() reloads the object from the StorageManager */
void MeasureGradient::load(Advocate & adv)
{
  GradientImplementation::load(adv);
  adv.loadAttribute("evaluation_", evaluation_);
  adv.loadAttribute("collection_", collection_);
  buildComponentGradients();
}


} /* namespace OTROBOPT */
