//                                               -*- C++ -*-
/**
 *  @brief Measure discretization
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
#include "otrobopt/MeasureFactory.hxx"
#include <otrobopt/MeasureEvaluationImplementation.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/UserDefined.hxx>
#include <openturns/LHSExperiment.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(MeasureFactory)

static Factory<MeasureFactory> Factory_MeasureFactory;


/* Default constructor */
MeasureFactory::MeasureFactory()
  : PersistentObject()
{
  // Nothing to do
}

/* Parameter constructor */
MeasureFactory::MeasureFactory (const WeightedExperiment & experiment)
  : PersistentObject()
  , experiment_(experiment)

{
  // Nothing to do
}

/* Virtual constructor method */
MeasureFactory * MeasureFactory::clone() const
{
  return new MeasureFactory(*this);
}


/* Evaluation */
MeasureEvaluation MeasureFactory::build(const MeasureEvaluation & measure) const
{
  // copy experiment as generate is non-const
  WeightedExperiment experimentCopy(experiment_);
  experimentCopy.setDistribution(measure.getDistribution());
  Point weights;
  const Sample sample(experimentCopy.generateWithWeights(weights));
  MeasureEvaluation result(measure);
  result.setDistribution(UserDefined(sample, weights));
  return result;
}


MeasureFactory::MeasureEvaluationCollection MeasureFactory::buildCollection(const MeasureEvaluationCollection & collection) const
{
  const UnsignedInteger size = collection.getSize();
  if (size == 0) throw InvalidArgumentException(HERE) << "The collection cannot be empty";
  const Distribution distribution(collection[0].getDistribution());
  for (UnsignedInteger i = 1; i < size; ++ i)
  {
    if (collection[i].getDistribution() != distribution) throw InvalidArgumentException(HERE) << "Cannot discretize measures with different distributions";
  }

  // copy experiment as generate is non-const
  WeightedExperiment experimentCopy(experiment_);
  experimentCopy.setDistribution(distribution);
  Point weights;
  const Sample sample(experimentCopy.generateWithWeights(weights));
  // We build the common discretized distribution once and as a Distribution
  // in order to make the different copies to share the same implementation
  const Distribution discretizedDistribution(UserDefined(sample, weights));
  MeasureEvaluationCollection result(collection);
  for (UnsignedInteger i = 0; i < size; ++ i)
  {
    result[i].setDistribution(discretizedDistribution);
  }
  return result;
}


/* String converter */
String MeasureFactory::__repr__() const
{
  OSS oss;
  oss << "class=" << MeasureFactory::GetClassName();
  return oss;
}

/* Method save() stores the object through the StorageManager */
void MeasureFactory::save(Advocate & adv) const
{
  PersistentObject::save(adv);
  adv.saveAttribute("experiment_", experiment_);
}

/* Method load() reloads the object from the StorageManager */
void MeasureFactory::load(Advocate & adv)
{
  PersistentObject::load(adv);
  adv.loadAttribute("experiment_", experiment_);
}


} /* namespace OTROBOPT */
