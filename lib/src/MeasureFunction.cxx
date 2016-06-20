//                                               -*- C++ -*-
/**
 *  @brief Measure function
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
#include "otrobopt/MeasureFunction.hxx"
#include "otrobopt/MeanMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/ResourceMap.hxx>
#include <openturns/GaussKronrodRule.hxx>

using namespace OT;

namespace OTROBOPT
{


CLASSNAMEINIT(MeasureFunction);

static Factory<MeasureFunction> Factory_MeasureFunction;

/* Default constructor */
MeasureFunction::MeasureFunction()
  : NumericalMathFunctionImplementation()
{
  // Nothing to do
}

/* Default constructor */
MeasureFunction::MeasureFunction(const MeasureEvaluation & evaluation)
: NumericalMathFunctionImplementation(evaluation.getImplementation())
, evaluation_(evaluation)
{
  // Nothing to do
}


MeasureFunction * MeasureFunction::clone() const
{
  return new MeasureFunction(*this);
}


/* Evaluation */
NumericalPoint MeasureFunction::operator()(const NumericalPoint & inP) const
{
  return evaluation_.operator()(inP);
}

NumericalSample MeasureFunction::operator()(const NumericalSample & inS) const
{
  const UnsignedInteger size = inS.getSize();
  NumericalSample outS(size, getOutputDimension());
  for (UnsignedInteger i = 0; i < size; ++ i)
  {
    outS[i] = operator()(inS[i]);
  }
  return outS;
}


UnsignedInteger MeasureFunction::getInputDimension() const
{
  return evaluation_.getImplementation()->getInputDimension();
}

UnsignedInteger MeasureFunction::getOutputDimension() const
{
  return evaluation_.getImplementation()->getOutputDimension();
}

/* String converter */
String MeasureFunction::__repr__() const
{
  OSS oss;
  oss << "class=" << MeasureFunction::GetClassName()
      << " evaluation=" << evaluation_;
  return oss;
}
/* Method save() stores the object through the StorageManager */
void MeasureFunction::save(Advocate & adv) const
{
  NumericalMathFunctionImplementation::save(adv);
  adv.saveAttribute("evaluation_", evaluation_);
}

/* Method load() reloads the object from the StorageManager */
void MeasureFunction::load(Advocate & adv)
{
  NumericalMathFunctionImplementation::load(adv);
  adv.loadAttribute("evaluation_", evaluation_);
}

} /* namespace OTROBOPT */
