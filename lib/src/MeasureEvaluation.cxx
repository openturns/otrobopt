//                                               -*- C++ -*-
/**
 *  @brief Measure function evaluation
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
#include "otrobopt/MeasureEvaluation.hxx"
#include "otrobopt/MeanMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/ResourceMap.hxx>
#include <openturns/GaussKronrodRule.hxx>

using namespace OT;

namespace OTROBOPT
{


static pthread_mutex_t OTRobOptResourceMap_InstanceMutex_;
static UnsignedInteger OTRobOptResourceMap_initialized_ = 0;

class OTRobOptResourceMap_init
{
public:
OTRobOptResourceMap_init()
{

  if (!OTRobOptResourceMap_initialized_)
  {
#ifndef OT_MUTEXINIT_NOCHECK
    pthread_mutexattr_t attr;
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init(&OTRobOptResourceMap_InstanceMutex_, &attr);
#else
    pthread_mutex_init(&OTRobOptResourceMap_InstanceMutex_, NULL);
#endif
    // SequentialMonteCarloRobustAlgorithm
    ResourceMap::SetAsNumericalScalar("SequentialMonteCarloRobustAlgorithm-ConvergenceFactor", 1e-2);
    ResourceMap::SetAsUnsignedInteger("SequentialMonteCarloRobustAlgorithm-DefaultInitialSamplingSize", 10);

    ResourceMap::SetAsUnsignedInteger("MeanMeasure-GaussKronrodRule", GaussKronrodRule::G7K15);
    ResourceMap::SetAsUnsignedInteger("VarianceMeasure-GaussKronrodRule", GaussKronrodRule::G7K15);
    ResourceMap::SetAsUnsignedInteger("MeanStandardDeviationTradeoffMeasure-GaussKronrodRule", GaussKronrodRule::G7K15);
    ResourceMap::SetAsUnsignedInteger("QuantileMeasure-GaussKronrodRule", GaussKronrodRule::G7K15);

    ResourceMap::SetAsUnsignedInteger("IndividualChanceMeasure-GaussKronrodRule", GaussKronrodRule::G7K15);
    ResourceMap::SetAsUnsignedInteger("JointChanceMeasure-GaussKronrodRule", GaussKronrodRule::G7K15);
    OTRobOptResourceMap_initialized_ = 1;
  }
  assert(OTRobOptResourceMap_initialized_);
}

~OTRobOptResourceMap_init()
{
  if (OTRobOptResourceMap_initialized_)
    pthread_mutex_destroy(&OTRobOptResourceMap_InstanceMutex_);
  OTRobOptResourceMap_initialized_ = 0;
}

};

static const OTRobOptResourceMap_init static_initializer_OTRobOptResourceMap;

CLASSNAMEINIT(MeasureEvaluation);

/* Default constructor */
MeasureEvaluation::MeasureEvaluation()
  : TypedInterfaceObject<MeasureEvaluationImplementation>(new MeanMeasure)
{
  // Nothing to do
}

/* Default constructor */
MeasureEvaluation::MeasureEvaluation(const MeasureEvaluationImplementation & implementation)
: TypedInterfaceObject<MeasureEvaluationImplementation>(implementation.clone())
{
  // Nothing to do
}

/* Evaluation */
NumericalPoint MeasureEvaluation::operator()(const NumericalPoint & inP) const
{
  return getImplementation()->operator()(inP);
}

/* Distribution accessor */
void MeasureEvaluation::setDistribution(const Distribution & distribution)
{
  copyOnWrite();
  getImplementation()->setDistribution(distribution);
}

Distribution MeasureEvaluation::getDistribution() const
{
  return getImplementation()->getDistribution();
}

/* Function accessor */
void MeasureEvaluation::setFunction(const NumericalMathFunction & function)
{
  copyOnWrite();
  getImplementation()->setFunction(function);
}

NumericalMathFunction MeasureEvaluation::getFunction() const
{
  return getImplementation()->getFunction();
}

/* String converter */
String MeasureEvaluation::__repr__() const
{
  OSS oss;
  oss << "class=" << MeasureEvaluation::GetClassName()
      << " implementation=" << getImplementation()->__repr__();
  return oss;
}


} /* namespace OTROBOPT */
