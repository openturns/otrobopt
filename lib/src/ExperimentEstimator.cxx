//                                               -*- C++ -*-
/**
 *  @brief Experiment estimator of a system-event probability
 *
 *  Copyright 2005-2026 Airbus-EDF-IMACS-ONERA-Phimeca
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
#include "otrobopt/ExperimentEstimator.hxx"
#include "otrobopt/ProbabilityEstimatorTools.hxx"
#include <openturns/Exception.hxx>
#include <openturns/MonteCarloExperiment.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/SpecFunc.hxx>

#include <cmath>
#include <limits>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(ExperimentEstimator)

static Factory<ExperimentEstimator> Factory_ExperimentEstimator;

/* Default constructor */
ExperimentEstimator::ExperimentEstimator()
  : ProbabilityEstimatorImplementation()
  , experiment_(MonteCarloExperiment())
{
  // Nothing to do
}

/* Constructor from a weighted experiment */
ExperimentEstimator::ExperimentEstimator(const WeightedExperiment & experiment)
  : ProbabilityEstimatorImplementation()
  , experiment_(experiment)
{
  // Nothing to do
}

/* Virtual constructor method */
ExperimentEstimator * ExperimentEstimator::clone() const
{
  return new ExperimentEstimator(*this);
}

/* Experiment accessor */
void ExperimentEstimator::setExperiment(const WeightedExperiment & experiment)
{
  experiment_ = experiment;
}

WeightedExperiment ExperimentEstimator::getExperiment() const
{
  return experiment_;
}

/* Probability and normalized empirical weights */
Scalar ExperimentEstimator::evaluate(const Function & predicate,
                                     const Function & limitState,
                                     const Distribution & distribution,
                                     const Point & designPoint,
                                     Point & weights) const
{
  WeightedExperiment experiment(experiment_);
  experiment.setDistribution(distribution);
  const Sample sample(experiment.generateWithWeights(weights));
  const UnsignedInteger size = sample.getSize();
  if (size == 0)
    throw InvalidArgumentException(HERE) << "The experiment generated an empty sample";
  if (weights.getSize() != size)
    throw InvalidArgumentException(HERE) << "The experiment generated " << size << " points but "
                                         << weights.getSize() << " weights";
  Scalar weightSum = 0.0;
  for (UnsignedInteger i = 0; i < size; ++i)
  {
    if (!(weights[i] >= 0.0))
      throw InvalidArgumentException(HERE) << "The experiment generated a negative weight " << weights[i];
    weightSum += weights[i];
  }
  if (!(weightSum > 0.0))
    throw InvalidArgumentException(HERE) << "The experiment generated a zero total weight";
  for (UnsignedInteger i = 0; i < size; ++i)
    weights[i] /= weightSum;

  Scalar probability = 0.0;
  for (UnsignedInteger i = 0; i < size; ++i)
  {
    const Point value(ProbabilityEstimatorTools::evaluateLimitState(limitState, designPoint, sample[i]));
    const Point indicators(ProbabilityEstimatorTools::buildIndicators(value));
    if (ProbabilityEstimatorTools::evaluatePredicate(predicate, indicators) != 0.0)
      probability += weights[i];
  }
  return probability;
}

/* Probability implementation */
Scalar ExperimentEstimator::computeSafeProbability(const Function & predicate,
                                                   const Function & limitState,
                                                   const Distribution & distribution,
                                                   const Point & designPoint) const
{
  Point weights;
  return evaluate(predicate, limitState, distribution, designPoint, weights);
}

/* Error implementation */
Scalar ExperimentEstimator::computeEstimationError(const Function & predicate,
                                                   const Function & limitState,
                                                   const Distribution & distribution,
                                                   const Point & designPoint) const
{
  Point weights;
  const Scalar probability = evaluate(predicate, limitState, distribution, designPoint, weights);
  Scalar squaredWeights = 0.0;
  for (UnsignedInteger i = 0; i < weights.getSize(); ++i)
    squaredWeights += weights[i] * weights[i];
  if (!(squaredWeights > 0.0))
    return std::numeric_limits<Scalar>::quiet_NaN();
  const Scalar effectiveSize = 1.0 / squaredWeights;
  // With one effective draw the variance cannot be estimated from the sample.
  if (!(effectiveSize > 1.0))
    return std::numeric_limits<Scalar>::quiet_NaN();
  const Scalar clippedProbability = SpecFunc::Clip01(probability);
  return std::sqrt(clippedProbability * (1.0 - clippedProbability) / effectiveSize);
}

/* String converter */
String ExperimentEstimator::__repr__() const
{
  OSS oss;
  oss << "class=" << ExperimentEstimator::GetClassName()
      << " experiment=" << experiment_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void ExperimentEstimator::save(Advocate & adv) const
{
  ProbabilityEstimatorImplementation::save(adv);
  adv.saveAttribute("experiment_", experiment_);
}

/* Method load() reloads the object from the StorageManager */
void ExperimentEstimator::load(Advocate & adv)
{
  ProbabilityEstimatorImplementation::load(adv);
  adv.loadAttribute("experiment_", experiment_);
}

} /* namespace OTROBOPT */
