//                                               -*- C++ -*-
/**
 *  @brief Measure function evaluation
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
#include "otrobopt/MeasureEvaluation.hxx"
#include "otrobopt/MeanMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/ResourceMap.hxx>
#include <openturns/GaussKronrodRule.hxx>
#include <mutex>

using namespace OT;

namespace OTROBOPT
{

struct OTRobOptResourceMap_init
{
  OTRobOptResourceMap_init()
  {
    static std::once_flag flag;
    std::call_once(flag, [&]()
    {
      ResourceMap::AddAsScalar("SequentialMonteCarloRobustAlgorithm-ConvergenceFactor", 1e-2);
      ResourceMap::AddAsUnsignedInteger("SequentialMonteCarloRobustAlgorithm-DefaultInitialSamplingSize", 10);

      ResourceMap::AddAsUnsignedInteger("MeanMeasure-GaussKronrodRule", GaussKronrodRule::G7K15);
      ResourceMap::AddAsUnsignedInteger("VarianceMeasure-GaussKronrodRule", GaussKronrodRule::G7K15);
      ResourceMap::AddAsUnsignedInteger("MeanStandardDeviationTradeoffMeasure-GaussKronrodRule", GaussKronrodRule::G7K15);
      ResourceMap::AddAsUnsignedInteger("QuantileMeasure-GaussKronrodRule", GaussKronrodRule::G7K15);

      ResourceMap::AddAsUnsignedInteger("IndividualChanceMeasure-GaussKronrodRule", GaussKronrodRule::G7K15);
      ResourceMap::AddAsUnsignedInteger("JointChanceMeasure-GaussKronrodRule", GaussKronrodRule::G7K15);
    });
  }
};

static const OTRobOptResourceMap_init __OTRobOptResourceMap_initializer;

CLASSNAMEINIT(MeasureEvaluation)

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
Point MeasureEvaluation::operator()(const Point & inP) const
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
void MeasureEvaluation::setFunction(const Function & function)
{
  copyOnWrite();
  getImplementation()->setFunction(function);
}

Function MeasureEvaluation::getFunction() const
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
