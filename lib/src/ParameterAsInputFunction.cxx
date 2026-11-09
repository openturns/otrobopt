//                                               -*- C++ -*-
/**
 *  @brief Function whose parameter is exposed as input coordinates
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
#include "otrobopt/ParameterAsInputFunction.hxx"

#include <openturns/Description.hxx>
#include <openturns/Exception.hxx>
#include <openturns/PersistentObjectFactory.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(ParameterAsInputFunction)

static Factory<ParameterAsInputFunction> Factory_ParameterAsInputFunction;

/* Default constructor */
ParameterAsInputFunction::ParameterAsInputFunction()
  : FunctionImplementation()
{
  // Nothing to do
}

/* Parameter constructor */
ParameterAsInputFunction::ParameterAsInputFunction(const Function & function)
  : FunctionImplementation()
  , function_(function)
{
  initializeDescriptions();
}

void ParameterAsInputFunction::initializeDescriptions()
{
  const UnsignedInteger inputDimension = function_.getInputDimension();
  const UnsignedInteger parameterDimension = function_.getParameterDimension();
  const UnsignedInteger outputDimension = function_.getOutputDimension();
  inputDescription_ = function_.getInputDescription();
  if (inputDescription_.getSize() != inputDimension)
    inputDescription_ = Description::BuildDefault(inputDimension, "x");
  Description parameterDescription(function_.getParameterDescription());
  if (parameterDescription.getSize() != parameterDimension)
    parameterDescription = Description::BuildDefault(parameterDimension, "theta");
  inputDescription_.add(parameterDescription);
  outputDescription_ = function_.getOutputDescription();
  if (outputDescription_.getSize() != outputDimension)
    outputDescription_ = Description::BuildDefault(outputDimension, "y");
}

/* Virtual constructor */
ParameterAsInputFunction * ParameterAsInputFunction::clone() const
{
  return new ParameterAsInputFunction(*this);
}

/* Evaluation */
Point ParameterAsInputFunction::operator()(const Point & inP) const
{
  const UnsignedInteger inputDimension = function_.getInputDimension();
  const UnsignedInteger parameterDimension = function_.getParameterDimension();
  const UnsignedInteger totalDimension = inputDimension + parameterDimension;
  if (inP.getDimension() != totalDimension)
    throw InvalidArgumentException(HERE) << "Error: expected a point of dimension "
                                         << totalDimension << ", got "
                                         << inP.getDimension();
  const Point u(inP.begin(), inP.begin() + inputDimension);
  const Point theta(inP.begin() + inputDimension, inP.end());
  Function function(function_);
  function.setParameter(theta);
  ++callsNumber_;
  return function(u);
}

Sample ParameterAsInputFunction::operator()(const Sample & inS) const
{
  const UnsignedInteger inputDimension = function_.getInputDimension();
  const UnsignedInteger parameterDimension = function_.getParameterDimension();
  const UnsignedInteger totalDimension = inputDimension + parameterDimension;
  if (inS.getDimension() != totalDimension)
    throw InvalidArgumentException(HERE) << "Error: expected a sample of dimension "
                                         << totalDimension << ", got "
                                         << inS.getDimension();
  const UnsignedInteger size = inS.getSize();
  Sample outS(size, getOutputDimension());
  for (UnsignedInteger i = 0; i < size; ++i)
    outS[i] = operator()(inS[i]);
  return outS;
}

/* Gradient against the input coordinates */
Matrix ParameterAsInputFunction::gradient(const Point & inP) const
{
  const UnsignedInteger inputDimension = function_.getInputDimension();
  const UnsignedInteger parameterDimension = function_.getParameterDimension();
  const UnsignedInteger outputDimension = function_.getOutputDimension();
  const UnsignedInteger totalDimension = inputDimension + parameterDimension;
  if (inP.getDimension() != totalDimension)
    throw InvalidArgumentException(HERE) << "Error: expected a point of dimension "
                                         << totalDimension << ", got "
                                         << inP.getDimension();
  const Point u(inP.begin(), inP.begin() + inputDimension);
  const Point theta(inP.begin() + inputDimension, inP.end());

  Function function(function_);
  function.setParameter(theta);
  const Matrix inputGradient(function.gradient(u));
  const Matrix parameterGradient(function.parameterGradient(u));

  Matrix gradient(outputDimension, totalDimension);
  for (UnsignedInteger i = 0; i < outputDimension; ++i)
    for (UnsignedInteger j = 0; j < inputDimension; ++j)
      gradient(i, j) = inputGradient(i, j);
  for (UnsignedInteger i = 0; i < outputDimension; ++i)
    for (UnsignedInteger j = 0; j < parameterDimension; ++j)
      gradient(i, inputDimension + j) = parameterGradient(j, i);
  return gradient;
}

/* Gradient against the parameter: empty, the parameter is now an input */
Matrix ParameterAsInputFunction::parameterGradient(const Point & inP) const
{
  const UnsignedInteger totalDimension = function_.getInputDimension() + function_.getParameterDimension();
  if (inP.getDimension() != totalDimension)
    throw InvalidArgumentException(HERE) << "Error: expected a point of dimension "
                                         << totalDimension << ", got "
                                         << inP.getDimension();
  return Matrix(0, function_.getOutputDimension());
}

void ParameterAsInputFunction::setParameter(const Point & parameter)
{
  if (parameter.getDimension() != 0)
    throw InvalidArgumentException(HERE) << "Error: ParameterAsInputFunction has no parameter, got a parameter of dimension "
                                         << parameter.getDimension();
  FunctionImplementation::setParameter(parameter);
}

/* Dimension accessors */
UnsignedInteger ParameterAsInputFunction::getInputDimension() const
{
  return function_.getInputDimension() + function_.getParameterDimension();
}

UnsignedInteger ParameterAsInputFunction::getOutputDimension() const
{
  return function_.getOutputDimension();
}

/* Description accessors */
Description ParameterAsInputFunction::getInputDescription() const
{
  if (inputDescription_.getSize() == 0)
    return Description::BuildDefault(getInputDimension(), "x");
  return inputDescription_;
}

Description ParameterAsInputFunction::getOutputDescription() const
{
  if (outputDescription_.getSize() == 0)
    return Description::BuildDefault(getOutputDimension(), "y");
  return outputDescription_;
}

/* Function accessor */
Function ParameterAsInputFunction::getFunction() const
{
  return function_;
}

/* String converter */
String ParameterAsInputFunction::__repr__() const
{
  OSS oss;
  oss << "class=" << ParameterAsInputFunction::GetClassName()
      << " function=" << function_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void ParameterAsInputFunction::save(Advocate & adv) const
{
  FunctionImplementation::save(adv);
  adv.saveAttribute("function_", function_);
  adv.saveAttribute("inputDescription_", inputDescription_);
  adv.saveAttribute("outputDescription_", outputDescription_);
}

/* Method load() reloads the object from the StorageManager */
void ParameterAsInputFunction::load(Advocate & adv)
{
  FunctionImplementation::load(adv);
  adv.loadAttribute("function_", function_);
  if (adv.hasAttribute("inputDescription_"))
    adv.loadAttribute("inputDescription_", inputDescription_);
  if (adv.hasAttribute("outputDescription_"))
    adv.loadAttribute("outputDescription_", outputDescription_);
  if (inputDescription_.getSize() != getInputDimension()
      || outputDescription_.getSize() != getOutputDimension())
    initializeDescriptions();
}

} /* namespace OTROBOPT */
