//                                               -*- C++ -*-
/**
 *  @brief Measure discretization
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
#include "otrobopt/MeasureFactory.hxx"
#include <otrobopt/MeasureEvaluationImplementation.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/UserDefined.hxx>
#include <openturns/LHSExperiment.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(MeasureFactory);

static Factory<MeasureFactory> RegisteredFactory;


/* Default constructor */
MeasureFactory::MeasureFactory()
  : PersistentObject()
  , p_experiment_(0)
{
  // Nothing to do
}

/* Parameter constructor */
MeasureFactory::MeasureFactory (const MeasureEvaluation & measure,
                                const OT::WeightedExperiment & experiment)
  : PersistentObject()
  , measure_(measure)
  , p_experiment_(experiment.clone())

{
  // Nothing to do
}

/* Virtual constructor method */
MeasureFactory * MeasureFactory::clone() const
{
  return new MeasureFactory(*this);
}


/* Evaluation */
MeasureEvaluation MeasureFactory::build() const
{
  // copy experiment as generate is non-const
  Pointer<WeightedExperiment> p_experiment(p_experiment_->clone());
  NumericalPoint weights;
  NumericalSample sample(p_experiment->generateWithWeights(weights));
  MeasureEvaluation measure(measure_);
  measure.setDistribution(UserDefined(sample, weights));
  return measure;
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
  adv.saveAttribute("measure_", measure_);
  adv.saveAttribute("experiment_", *p_experiment_);
}

/* Method load() reloads the object from the StorageManager */
void MeasureFactory::load(Advocate & adv)
{
  PersistentObject::load(adv);
  adv.loadAttribute("measure_", measure_);
  p_experiment_ = new WeightedExperiment;
  adv.loadAttribute("experiment_", *p_experiment_);
}


} /* namespace OTROBOPT */
