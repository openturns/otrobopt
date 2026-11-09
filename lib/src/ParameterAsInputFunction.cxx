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

#include <openturns/CenteredFiniteDifferenceHessian.hxx>
#include <openturns/Description.hxx>
#include <openturns/EvaluationImplementation.hxx>
#include <openturns/Exception.hxx>
#include <openturns/GradientImplementation.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/ResourceMap.hxx>

using namespace OT;

namespace OTROBOPT
{

/* Gradient of H(u, theta) = f_theta(u) against the coordinates of H */
static Matrix gradientParameterAsInput(const Function & function,
                                       const Point & inP)
{
  const UnsignedInteger inputDimension = function.getInputDimension();
  const UnsignedInteger parameterDimension = function.getParameterDimension();
  const UnsignedInteger outputDimension = function.getOutputDimension();
  const UnsignedInteger totalDimension = inputDimension + parameterDimension;
  if (inP.getDimension() != totalDimension)
    throw InvalidArgumentException(HERE) << "Error: expected a point of dimension "
                                         << totalDimension << ", got "
                                         << inP.getDimension();
  const Point u(inP.begin(), inP.begin() + inputDimension);
  const Point theta(inP.begin() + inputDimension, inP.end());

  Function functionCopy(function);
  functionCopy.setParameter(theta);
  const Matrix inputGradient(functionCopy.gradient(u));
  const Matrix parameterGradient(functionCopy.parameterGradient(u));

  Matrix gradient(totalDimension, outputDimension);
  for (UnsignedInteger i = 0; i < outputDimension; ++i)
    for (UnsignedInteger j = 0; j < inputDimension; ++j)
      gradient(j, i) = inputGradient(j, i);
  for (UnsignedInteger i = 0; i < outputDimension; ++i)
    for (UnsignedInteger j = 0; j < parameterDimension; ++j)
      gradient(inputDimension + j, i) = parameterGradient(j, i);
  return gradient;
}

static Description parameterAsInputDescription(const Function & function)
{
  const UnsignedInteger inputDimension = function.getInputDimension();
  const UnsignedInteger parameterDimension = function.getParameterDimension();
  Description inputDescription(function.getInputDescription());
  if (inputDescription.getSize() != inputDimension)
    inputDescription = Description::BuildDefault(inputDimension, "x");
  Description parameterDescription(function.getParameterDescription());
  if (parameterDescription.getSize() != parameterDimension)
    parameterDescription = Description::BuildDefault(parameterDimension, "theta");
  inputDescription.add(parameterDescription);
  return inputDescription;
}

static Description parameterAsInputOutputDescription(const Function & function)
{
  const UnsignedInteger outputDimension = function.getOutputDimension();
  Description outputDescription(function.getOutputDescription());
  if (outputDescription.getSize() != outputDimension)
    outputDescription = Description::BuildDefault(outputDimension, "y");
  return outputDescription;
}

/* Internal evaluation used when this function is composed with another one */
class ParameterAsInputEvaluation
  : public EvaluationImplementation
{
  CLASSNAME

public:
  ParameterAsInputEvaluation()
    : EvaluationImplementation()
  {
    // Nothing to do
  }

  explicit ParameterAsInputEvaluation(const Function & function)
    : EvaluationImplementation()
    , function_(function)
  {
    // Nothing to do
  }

  ParameterAsInputEvaluation * clone() const override
  {
    return new ParameterAsInputEvaluation(*this);
  }

  Point operator()(const Point & inP) const override
  {
    ++callsNumber_;
    // H(u, theta) = f_theta(u): split the coordinates of H into the input
    // point and the parameter of f, then evaluate f with that parameter.
    const UnsignedInteger inputDimension = function_.getInputDimension();
    const UnsignedInteger parameterDimension = function_.getParameterDimension();
    const UnsignedInteger totalDimension = inputDimension + parameterDimension;
    if (inP.getDimension() != totalDimension)
      throw InvalidArgumentException(HERE) << "Error: expected a point of dimension "
                                           << totalDimension << ", got "
                                           << inP.getDimension();
    const Point u(inP.begin(), inP.begin() + inputDimension);
    const Point theta(inP.begin() + inputDimension, inP.end());
    Function functionCopy(function_);
    functionCopy.setParameter(theta);
    return functionCopy(u);
  }

  Matrix parameterGradient(const Point & inP) const override
  {
    const UnsignedInteger totalDimension = function_.getInputDimension() + function_.getParameterDimension();
    if (inP.getDimension() != totalDimension)
      throw InvalidArgumentException(HERE) << "Error: expected a point of dimension "
                                           << totalDimension << ", got "
                                           << inP.getDimension();
    return Matrix(0, function_.getOutputDimension());
  }

  UnsignedInteger getInputDimension() const override
  {
    return function_.getInputDimension() + function_.getParameterDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return function_.getOutputDimension();
  }

  Description getInputDescription() const override
  {
    return parameterAsInputDescription(function_);
  }

  Description getOutputDescription() const override
  {
    return parameterAsInputOutputDescription(function_);
  }

  void save(Advocate & adv) const override
  {
    EvaluationImplementation::save(adv);
    adv.saveAttribute("function_", function_);
  }

  void load(Advocate & adv) override
  {
    EvaluationImplementation::load(adv);
    adv.loadAttribute("function_", function_);
  }

private:
  Function function_;
};

/* Internal gradient used when this function is composed with another one */
class ParameterAsInputGradient
  : public GradientImplementation
{
  CLASSNAME

public:
  ParameterAsInputGradient()
    : GradientImplementation()
  {
    // Nothing to do
  }

  explicit ParameterAsInputGradient(const Function & function)
    : GradientImplementation()
    , function_(function)
  {
    // Nothing to do
  }

  ParameterAsInputGradient * clone() const override
  {
    return new ParameterAsInputGradient(*this);
  }

  Matrix gradient(const Point & inP) const override
  {
    ++callsNumber_;
    return gradientParameterAsInput(function_, inP);
  }

  UnsignedInteger getInputDimension() const override
  {
    return function_.getInputDimension() + function_.getParameterDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return function_.getOutputDimension();
  }

  void save(Advocate & adv) const override
  {
    GradientImplementation::save(adv);
    adv.saveAttribute("function_", function_);
  }

  void load(Advocate & adv) override
  {
    GradientImplementation::load(adv);
    adv.loadAttribute("function_", function_);
  }

private:
  Function function_;
};

CLASSNAMEINIT(ParameterAsInputEvaluation)

static Factory<ParameterAsInputEvaluation> Factory_ParameterAsInputEvaluation;

CLASSNAMEINIT(ParameterAsInputGradient)

static Factory<ParameterAsInputGradient> Factory_ParameterAsInputGradient;

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
  setEvaluation(Evaluation(new ParameterAsInputEvaluation(function_)));
  setGradient(Gradient(new ParameterAsInputGradient(function_)));
  // Older OpenTURNS versions leave a NoHessian here, which reports a zero
  // output dimension and makes the construction of a marginal of a
  // vector-valued wrapper fail.
  setHessian(new CenteredFiniteDifferenceHessian(
      ResourceMap::GetAsScalar("CenteredFiniteDifferenceHessian-DefaultEpsilon"),
      getEvaluation()));
}

void ParameterAsInputFunction::initializeDescriptions()
{
  inputDescription_ = parameterAsInputDescription(function_);
  outputDescription_ = parameterAsInputOutputDescription(function_);
}

/* Virtual constructor */
ParameterAsInputFunction * ParameterAsInputFunction::clone() const
{
  return new ParameterAsInputFunction(*this);
}

/* Gradient against the input coordinates */
Matrix ParameterAsInputFunction::gradient(const Point & inP) const
{
  return gradientParameterAsInput(function_, inP);
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
  // Keep the composition delegates consistent with the reloaded function
  setEvaluation(Evaluation(new ParameterAsInputEvaluation(function_)));
  setGradient(Gradient(new ParameterAsInputGradient(function_)));
}

} /* namespace OTROBOPT */
