//                                               -*- C++ -*-
/**
 *  @brief Measure function evaluation implementation
 *
 *  Copyright 2005-2018 Airbus-EDF-IMACS-Phimeca
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
#include "otrobopt/MeasureEvaluationImplementation.hxx"
#include <openturns/PersistentObjectFactory.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(MeasureEvaluationImplementation)

static Factory<MeasureEvaluationImplementation> Factory_MeasureEvaluationImplementation;


/* Default constructor */
MeasureEvaluationImplementation::MeasureEvaluationImplementation()
  : EvaluationImplementation()
{
  // Nothing to do
}

MeasureEvaluationImplementation::MeasureEvaluationImplementation (const Function & function,
                                                                  const Distribution & distribution)
  : EvaluationImplementation()
  , function_(function)
  , distribution_(distribution)
{
  if (distribution.getDimension() != function.getParameter().getDimension())
    throw InvalidDimensionException(HERE) << "Function parameter dimension (" << function.getParameter().getDimension()
      << ") must match the distribution dimension (" << distribution.getDimension()<<")";
  setInputDescription(function_.getInputDescription());
  setOutputDescription(function_.getOutputDescription());
}


/* Virtual constructor method */
MeasureEvaluationImplementation * MeasureEvaluationImplementation::clone() const
{
  return new MeasureEvaluationImplementation(*this);
}


/* Evaluation */
Point MeasureEvaluationImplementation::operator()(const Point & inP) const
{
  throw NotYetImplementedException(HERE) << "MeasureEvaluationImplementation::operator()(const Point & inP)";
}


/* String converter */
String MeasureEvaluationImplementation::__repr__() const
{
  OSS oss;
  oss << "class=" << MeasureEvaluationImplementation::GetClassName()
      << " distribution="<< distribution_
      << " function=" << function_;
  return oss;
}

String MeasureEvaluationImplementation::__str__(const String & offset) const
{
  OSS oss;
  oss << offset << __repr__();
  return oss;
}


/* Distribution accessor */
void MeasureEvaluationImplementation::setDistribution(const Distribution & distribution)
{
  distribution_ = distribution;
}


Distribution MeasureEvaluationImplementation::getDistribution() const
{
  return distribution_;
}


/* Function accessor */
void MeasureEvaluationImplementation::setFunction(const Function & function)
{
  function_ = function;
}

Function MeasureEvaluationImplementation::getFunction() const
{
  return function_;
}


UnsignedInteger MeasureEvaluationImplementation::getInputDimension() const
{
  return function_.getInputDimension();
}

UnsignedInteger MeasureEvaluationImplementation::getOutputDimension() const
{
  return function_.getOutputDimension();
}

/* Method save() stores the object through the StorageManager */
void MeasureEvaluationImplementation::save(Advocate & adv) const
{
  EvaluationImplementation::save(adv);
  adv.saveAttribute("distribution_", distribution_);
  adv.saveAttribute("function_", function_);
}

/* Method load() reloads the object from the StorageManager */
void MeasureEvaluationImplementation::load(Advocate & adv)
{
  EvaluationImplementation::load(adv);
  adv.loadAttribute("distribution_", distribution_);
  adv.loadAttribute("function_", function_);
}


} /* namespace OTROBOPT */
