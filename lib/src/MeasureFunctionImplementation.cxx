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
