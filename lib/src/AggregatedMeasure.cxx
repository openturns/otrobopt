//                                               -*- C++ -*-
/**
 *  @brief Aggregation of measure functions
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
#include "otrobopt/AggregatedMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/AggregatedFunction.hxx>

using namespace OT;
namespace OT {
TEMPLATE_CLASSNAMEINIT(PersistentCollection<OTROBOPT::MeasureEvaluation>);
}

namespace OTROBOPT
{

CLASSNAMEINIT(AggregatedMeasure);

static Factory<AggregatedMeasure> Factory_AggregatedMeasure;

static const Factory<PersistentCollection<MeasureEvaluation> > Factory_PersistentCollection_MeasureEvaluation;

/* Default constructor */
AggregatedMeasure::AggregatedMeasure()
  : MeasureEvaluationImplementation()
{
  // Nothing to do
}

/* Parameter constructor */
AggregatedMeasure::AggregatedMeasure(const MeasureEvaluationCollection & collection)
  : MeasureEvaluationImplementation()
  , collection_(collection)
{
  const UnsignedInteger size = collection_.getSize();
  if (size == 0)
    throw InvalidArgumentException(HERE) << "Empty collection";
  Collection <NumericalMathFunction> funcColl(size);
  for (UnsignedInteger i = 0; i < size; ++ i)
    funcColl[i] = collection_[i].getFunction();
  setFunction(AggregatedFunction(funcColl));
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
    collection_[i].setDistribution(distribution);
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
  MeasureEvaluationImplementation::save(adv);
  adv.saveAttribute("collection_", collection_);
}

/* Method load() reloads the object from the StorageManager */
void AggregatedMeasure::load(Advocate & adv)
{
  MeasureEvaluationImplementation::load(adv);
  adv.loadAttribute("collection_", collection_);
}


} /* namespace OTROBOPT */
