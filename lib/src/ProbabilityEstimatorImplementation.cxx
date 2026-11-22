//                                               -*- C++ -*-
/**
 *  @brief Base class of the system-event probability estimators
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
#include "otrobopt/ProbabilityEstimatorImplementation.hxx"
#include "otrobopt/ProbabilityEstimatorTools.hxx"
#include <openturns/Exception.hxx>
#include <openturns/SpecFunc.hxx>

#include <cmath>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(ProbabilityEstimatorImplementation)

/* Default constructor */
ProbabilityEstimatorImplementation::ProbabilityEstimatorImplementation()
  : PersistentObject()
{
  // Nothing to do
}

/* Probability of the event at the design point */
Scalar ProbabilityEstimatorImplementation::getSafeProbability(const Function & predicate,
                                                              const Function & limitState,
                                                              const Distribution & distribution,
                                                              const Point & designPoint) const
{
  ProbabilityEstimatorTools::checkArguments(predicate, limitState, distribution, designPoint);
  Scalar probability = computeSafeProbability(predicate, limitState, distribution, designPoint);
  // Only the roundoff of an otherwise valid probability is clipped: a value
  // farther from [0, 1] is an estimator defect, not a numerical accident.
  if (!(probability >= -SpecFunc::Precision) || !(probability <= 1.0 + SpecFunc::Precision))
    throw InternalException(HERE) << "The estimator returned an invalid probability " << probability;
  probability = SpecFunc::Clip01(probability);
  return probability;
}

/* Error estimate of the probability, NaN if undefined */
Scalar ProbabilityEstimatorImplementation::getEstimationError(const Function & predicate,
                                                              const Function & limitState,
                                                              const Distribution & distribution,
                                                              const Point & designPoint) const
{
  ProbabilityEstimatorTools::checkArguments(predicate, limitState, distribution, designPoint);
  const Scalar error = computeEstimationError(predicate, limitState, distribution, designPoint);
  if (std::isnan(error))
    return error;
  if (!(error >= 0.0) || !std::isfinite(error))
    throw InternalException(HERE) << "The estimator returned an invalid error " << error;
  return error;
}

/* Whether the estimator provides a geometry */
Bool ProbabilityEstimatorImplementation::hasGeometry() const
{
  return false;
}

/* Signed reliability index of each limit-state component */
Point ProbabilityEstimatorImplementation::getComponentReliabilityIndexes(const Function & /*predicate*/,
                                                                          const Function & /*limitState*/,
                                                                          const Distribution & /*distribution*/,
                                                                          const Point & /*designPoint*/) const
{
  throw NotDefinedException(HERE) << "The estimator " << GetClassName() << " has no per-component reliability index";
}

/* Standard-space design points, grouped by limit-state component */
Sample ProbabilityEstimatorImplementation::getStandardDesignPoints(const Function & /*predicate*/,
                                                                   const Function & /*limitState*/,
                                                                   const Distribution & /*distribution*/,
                                                                   const Point & /*designPoint*/) const
{
  throw NotDefinedException(HERE) << "The estimator " << GetClassName() << " has no standard-space design point";
}

/* Number of standard-space design points of each limit-state component */
Indices ProbabilityEstimatorImplementation::getDesignPointsNumber(const Function & /*predicate*/,
                                                                  const Function & /*limitState*/,
                                                                  const Distribution & /*distribution*/,
                                                                  const Point & /*designPoint*/) const
{
  throw NotDefinedException(HERE) << "The estimator " << GetClassName() << " has no standard-space design point";
}

/* Bounds of the probability of the linearized event */
Interval ProbabilityEstimatorImplementation::getProbabilityBounds(const Function & /*predicate*/,
                                                                  const Function & /*limitState*/,
                                                                  const Distribution & /*distribution*/,
                                                                  const Point & /*designPoint*/) const
{
  throw NotDefinedException(HERE) << "The estimator " << GetClassName() << " has no probability bounds";
}

/* String converter */
String ProbabilityEstimatorImplementation::__repr__() const
{
  OSS oss;
  oss << "class=" << ProbabilityEstimatorImplementation::GetClassName();
  return oss;
}

} /* namespace OTROBOPT */
