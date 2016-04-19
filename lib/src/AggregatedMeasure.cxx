//                                               -*- C++ -*-
/**
 *  @brief Aggregation of measure functions
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
#include "otrobopt/AggregatedMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/IteratedQuadrature.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(AggregatedMeasure);

static Factory<AggregatedMeasure> RegisteredFactory;


/* Default constructor */
AggregatedMeasure::AggregatedMeasure()
  : MeasureFunctionImplementation()
{
  // Nothing to do
}

/* Parameter constructor */
AggregatedMeasure::AggregatedMeasure(const MeasureFunctionCollection & collection)
  : MeasureFunctionImplementation()
  , collection_(collection)
{
  const UnsignedInteger size = collection_.getSize();
  if (size == 0)
    throw InvalidArgumentException(HERE) << "Empty collection";
  Collection <NumericalMathFunction> funcColl(size);
  for (UnsignedInteger i = 0; i < size; ++ i)
  {
    funcColl[i] = collection_[i].getFunction();
  }
  setFunction(NumericalMathFunction(funcColl));
}

/* Virtual constructor method */
AggregatedMeasure * AggregatedMeasure::clone() const
{
  return new AggregatedMeasure(*this);
}

/* Evaluation */
NumericalPoint AggregatedMeasure::operator()(const NumericalPoint & inP) const
{
  const UnsignedInteger outputDimension = getFunction().getOutputDimension();
  NumericalPoint outP(outputDimension);
  const UnsignedInteger size = collection_.getSize();
  UnsignedInteger index = 0;
  for (UnsignedInteger i = 0; i < size; ++ i)
  {
    const NumericalPoint outPi(collection_[i](inP));
    std::copy(outPi.begin(), outPi.end(), outP.begin() + index);
    index += outPi.getSize();
  }
  return outP;
}


/* Distribution accessor */
void AggregatedMeasure::setDistribution(const Distribution & distribution)
{
  const UnsignedInteger size = collection_.getSize();
  for (UnsignedInteger i = 0; i < size; ++ i)
  {
    collection_[i].setDistribution(distribution);
  }
}


Distribution AggregatedMeasure::getDistribution() const
{
  const UnsignedInteger size = collection_.getSize();
  if (size == 0)
    throw InvalidArgumentException(HERE) << "Empty collection";
  return collection_[0].getDistribution();
}


/* String converter */
String AggregatedMeasure::__repr__() const
{
  OSS oss;
  oss << "class=" << AggregatedMeasure::GetClassName()
      << " collection=" << collection_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void AggregatedMeasure::save(Advocate & adv) const
{
  MeasureFunctionImplementation::save(adv);
}

/* Method load() reloads the object from the StorageManager */
void AggregatedMeasure::load(Advocate & adv)
{
  MeasureFunctionImplementation::load(adv);
}


} /* namespace OTROBOPT */
