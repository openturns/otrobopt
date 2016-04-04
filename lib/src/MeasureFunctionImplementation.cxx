//                                               -*- C++ -*-
/**
 *  @brief MeasureFunctionImplementation
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
#include "otrobopt/MeasureFunctionImplementation.hxx"
#include <openturns/PersistentObjectFactory.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(MeasureFunctionImplementation);

static Factory<MeasureFunctionImplementation> RegisteredFactory;


/* Default constructor */
MeasureFunctionImplementation::MeasureFunctionImplementation()
  : NumericalMathFunctionImplementation()
{
  // Nothing to do
}

MeasureFunctionImplementation::MeasureFunctionImplementation (const Distribution & distribution,
                                                              const NumericalMathFunction & function)
  : NumericalMathFunctionImplementation()
  , distribution_(distribution)
  , function_(function)
{
  if (distribution.getDimension() != function.getParameter().getDimension())
    throw InvalidDimensionException(HERE) << "Function parameter dimension (" << function.getParameter().getDimension()
      << ") must match the distribution dimension (" << distribution.getDimension()<<")";
}


/* Virtual constructor method */
MeasureFunctionImplementation * MeasureFunctionImplementation::clone() const
{
  return new MeasureFunctionImplementation(*this);
}


/* Evaluation */
NumericalPoint MeasureFunctionImplementation::operator()(const NumericalPoint & inP) const
{
  throw NotYetImplementedException(HERE) << "MeasureFunctionImplementation::operator()(const NumericalPoint & inP)";
}


/* Method gradient() returns the Jacobian transposed matrix of the function at point */
Matrix MeasureFunctionImplementation::gradient(const NumericalPoint & inP) const
{
  const UnsignedInteger dimension = getInputDimension();
  const UnsignedInteger outputDimension = getOutputDimension();
  const NumericalScalar h = ResourceMap::GetAsNumericalScalar("NonCenteredFiniteDifferenceGradient-DefaultEpsilon");

  NumericalSample inS(dimension + 1, inP);
  for (UnsignedInteger j = 0; j < dimension; ++ j)
    inS[1 + j][j] += h;

  NumericalSample outS(dimension + 1, outputDimension);
  for (UnsignedInteger i = 0; i < dimension + 1; ++ i)
    outS[i] = operator()(inS[i]);

  Matrix gradient(dimension, outputDimension);
  for (UnsignedInteger i = 0; i < dimension; ++ i)
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
      gradient(i, j) = (outS[i + 1][j] - outS[0][j]) / h;

  return gradient;
}



/* String converter */
String MeasureFunctionImplementation::__repr__() const
{
  OSS oss;
  oss << "class=" << MeasureFunctionImplementation::GetClassName()
      << " distribution="<< distribution_
      << " function=" << function_;
  return oss;
}

String MeasureFunctionImplementation::__str__(const String & offset) const
{
  OSS oss;
  oss << offset << __repr__();
  return oss;
}


/* Distribution accessor */
void MeasureFunctionImplementation::setDistribution(const Distribution & distribution)
{
  distribution_ = distribution;
}


Distribution MeasureFunctionImplementation::getDistribution() const
{
  return distribution_;
}


/* Function accessor */
void MeasureFunctionImplementation::setFunction(const NumericalMathFunction & function)
{
  function_ = function;
}

NumericalMathFunction MeasureFunctionImplementation::getFunction() const
{
  return function_;
}


UnsignedInteger MeasureFunctionImplementation::getInputDimension() const
{
  return function_.getInputDimension();
}

UnsignedInteger MeasureFunctionImplementation::getOutputDimension() const
{
  return function_.getOutputDimension();
}

/* Method save() stores the object through the StorageManager */
void MeasureFunctionImplementation::save(Advocate & adv) const
{
  NumericalMathFunctionImplementation::save(adv);
  adv.saveAttribute("distribution_", distribution_);
  adv.saveAttribute("function_", function_);
}

/* Method load() reloads the object from the StorageManager */
void MeasureFunctionImplementation::load(Advocate & adv)
{
  NumericalMathFunctionImplementation::load(adv);
  adv.loadAttribute("distribution_", distribution_);
  adv.loadAttribute("function_", function_);
}


} /* namespace OTROBOPT */
