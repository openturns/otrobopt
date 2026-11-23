//                                               -*- C++ -*-
/**
 *  @brief Reliability-based estimator of a system-event probability
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
#include "otrobopt/ReliabilityEstimator.hxx"
#include "otrobopt/ProbabilityEstimatorTools.hxx"
#include "otrobopt/RBDOAlgorithmTools.hxx"

#include <openturns/ComposedFunction.hxx>
#include <openturns/CorrelationMatrix.hxx>
#include <openturns/DistFunc.hxx>
#include <openturns/EvaluationImplementation.hxx>
#include <openturns/GradientImplementation.hxx>
#include <openturns/HessianImplementation.hxx>
#include <openturns/LinearCombinationFunction.hxx>
#include <openturns/Normal.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/ResourceMap.hxx>
#include <openturns/SpecFunc.hxx>

#include <algorithm>
#include <cmath>
#include <limits>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(ReliabilityEstimator)

static Factory<ReliabilityEstimator> Factory_ReliabilityEstimator;

namespace
{

/* Restart bulge of MultiFORM: h * (r^2 - ||u - centre||^2)^2 inside the ball */
class BulgeEvaluation
  : public EvaluationImplementation
{
public:
  BulgeEvaluation(const Point & centre,
                  const Scalar radius,
                  const Scalar height)
    : EvaluationImplementation()
    , centre_(centre)
    , radiusSquare_(radius * radius)
    , height_(height)
  {
    // Nothing to do
  }

  BulgeEvaluation * clone() const override
  {
    return new BulgeEvaluation(*this);
  }

  Point operator()(const Point & point) const override
  {
    const Scalar distanceSquare((point - centre_).normSquare());
    if (distanceSquare <= radiusSquare_)
    {
      const Scalar gap(radiusSquare_ - distanceSquare);
      return Point(1, height_ * gap * gap);
    }
    return Point(1, 0.0);
  }

  UnsignedInteger getInputDimension() const override
  {
    return centre_.getDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return 1;
  }

private:
  Point centre_;
  Scalar radiusSquare_ = 0.0;
  Scalar height_ = 0.0;
};

/* Gradient of the restart bulge */
class BulgeGradient
  : public GradientImplementation
{
public:
  BulgeGradient(const Point & centre,
                const Scalar radius,
                const Scalar height)
    : GradientImplementation()
    , centre_(centre)
    , radiusSquare_(radius * radius)
    , height_(height)
  {
    // Nothing to do
  }

  BulgeGradient * clone() const override
  {
    return new BulgeGradient(*this);
  }

  Matrix gradient(const Point & point) const override
  {
    const UnsignedInteger dimension = centre_.getDimension();
    const Point difference(point - centre_);
    const Scalar distanceSquare(difference.normSquare());
    Matrix result(dimension, 1);
    if (distanceSquare <= radiusSquare_)
    {
      const Scalar factor(-4.0 * height_ * (radiusSquare_ - distanceSquare));
      for (UnsignedInteger i = 0; i < dimension; ++i)
        result(i, 0) = factor * difference[i];
    }
    return result;
  }

  UnsignedInteger getInputDimension() const override
  {
    return centre_.getDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return 1;
  }

private:
  Point centre_;
  Scalar radiusSquare_ = 0.0;
  Scalar height_ = 0.0;
};

/* Hessian of the restart bulge */
class BulgeHessian
  : public HessianImplementation
{
public:
  BulgeHessian(const Point & centre,
               const Scalar radius,
               const Scalar height)
    : HessianImplementation()
    , centre_(centre)
    , radiusSquare_(radius * radius)
    , height_(height)
  {
    // Nothing to do
  }

  BulgeHessian * clone() const override
  {
    return new BulgeHessian(*this);
  }

  SymmetricTensor hessian(const Point & point) const override
  {
    const UnsignedInteger dimension = centre_.getDimension();
    const Point difference(point - centre_);
    const Scalar distanceSquare(difference.normSquare());
    SymmetricTensor result(dimension, 1);
    if (distanceSquare <= radiusSquare_)
    {
      const Scalar gap(radiusSquare_ - distanceSquare);
      for (UnsignedInteger i = 0; i < dimension; ++i)
        for (UnsignedInteger j = 0; j <= i; ++j)
          result(i, j, -8.0 * height_ * difference[i] * difference[j]
                  + ((i == j) ? -4.0 * height_ * gap : 0.0));
    }
    return result;
  }

  UnsignedInteger getInputDimension() const override
  {
    return centre_.getDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return 1;
  }

private:
  Point centre_;
  Scalar radiusSquare_ = 0.0;
  Scalar height_ = 0.0;
};

/* Number of atoms of a truth table over a given number of binary variables,
   or 0 when the table does not fit in an UnsignedInteger. */
UnsignedInteger truthTableSize(const UnsignedInteger variableNumber)
{
  if (variableNumber >= 8 * sizeof(UnsignedInteger) - 1)
    return 0;
  return UnsignedInteger(1) << variableNumber;
}

/* One group of parallel or anti-parallel half-spaces, seen as an interval on
   the common standard normal projection */
struct HalfSpaceGroup
{
  Point direction;
  Scalar lowerBound = -std::numeric_limits<Scalar>::infinity();
  Scalar upperBound = std::numeric_limits<Scalar>::infinity();
};

/* Number of set bits of an integer */
UnsignedInteger bitCount(UnsignedInteger value)
{
  UnsignedInteger count = 0;
  while (value != 0)
  {
    count += value & 1;
    value >>= 1;
  }
  return count;
}

/* Group the half-spaces whose normals are parallel or anti-parallel.
   Two such half-spaces constrain the same standard normal variable, so their
   conjunction is an interval.  Keeping them as separate variables would build
   a singular multinormal law, which OpenTURNS rejects when the correlation is
   exactly -1. */
Collection<HalfSpaceGroup> buildHalfSpaceGroups(const Sample & unitNormals,
                                                const Point & thresholds)
{
  const UnsignedInteger halfSpaceNumber = thresholds.getDimension();
  // The normals are built from unit design-point directions: a parallel or
  // anti-parallel pair agrees to the rounding error of the inner solves.
  const Scalar parallelTolerance = 1e-10;
  Indices assigned(halfSpaceNumber, 0);
  Collection<HalfSpaceGroup> groups;
  for (UnsignedInteger k = 0; k < halfSpaceNumber; ++k)
  {
    if (assigned[k] != 0)
      continue;
    HalfSpaceGroup group;
    group.direction = unitNormals[k];
    for (UnsignedInteger j = k; j < halfSpaceNumber; ++j)
    {
      if (assigned[j] != 0)
        continue;
      const Scalar cosine = group.direction.dot(unitNormals[j]);
      if (std::abs(cosine) < 1.0 - parallelTolerance)
        continue;
      assigned[j] = 1;
      if (cosine >= 0.0)
        group.upperBound = std::min(group.upperBound, thresholds[j]);
      else
        group.lowerBound = std::max(group.lowerBound, -thresholds[j]);
    }
    groups.add(group);
  }
  return groups;
}

/* Probability that every group interval holds, by the signed sum of the
   orthant probabilities of its upper and lower bounds. */
Scalar intervalProbability(const Collection<HalfSpaceGroup> & groups,
                           const UnsignedInteger maximumTermNumber)
{
  // An empty intersection contributes nothing.
  for (UnsignedInteger i = 0; i < groups.getSize(); ++i)
    if (groups[i].lowerBound > groups[i].upperBound)
      return 0.0;
  // Each group with a finite lower bound adds one subtraction term.
  Indices divisionRank(groups.getSize(), groups.getSize());
  UnsignedInteger divisionNumber = 0;
  for (UnsignedInteger i = 0; i < groups.getSize(); ++i)
    if (groups[i].lowerBound > -std::numeric_limits<Scalar>::infinity())
      divisionRank[i] = divisionNumber++;
  const UnsignedInteger termNumber = truthTableSize(divisionNumber);
  if ((termNumber == 0) || (termNumber > maximumTermNumber))
    throw NotYetImplementedException(HERE) << "The interval combination needs " << termNumber
                                           << " orthant terms, above the maximum of " << maximumTermNumber
                                           << "; use the SIMULATION system algorithm";
  Scalar probability = 0.0;
  for (UnsignedInteger mask = 0; mask < termNumber; ++mask)
  {
    Collection<UnsignedInteger> selected;
    Point selectedThresholds;
    for (UnsignedInteger i = 0; i < groups.getSize(); ++i)
    {
      const Bool subtract = (divisionRank[i] < divisionNumber) && (((mask >> divisionRank[i]) & 1) != 0);
      const Scalar bound = subtract ? groups[i].lowerBound : groups[i].upperBound;
      if (std::isfinite(bound))
      {
        selected.add(i);
        selectedThresholds.add(bound);
      }
    }
    const UnsignedInteger selectedNumber = selected.getSize();
    Scalar term = 1.0;
    if (selectedNumber > 0)
    {
      CorrelationMatrix correlation(selectedNumber);
      for (UnsignedInteger i = 0; i < selectedNumber; ++i)
        for (UnsignedInteger j = 0; j < i; ++j)
        {
          const Scalar rho = groups[selected[i]].direction.dot(groups[selected[j]].direction);
          correlation(i, j) = std::max(-1.0, std::min(1.0, rho));
        }
      if (selectedNumber == 1)
        term = DistFunc::pNormal(selectedThresholds[0]);
      else
        try
        {
          term = Normal(Point(selectedNumber), correlation).computeCDF(selectedThresholds);
        }
        catch (const Exception &)
        {
          throw NotYetImplementedException(HERE) << "The linearized half-spaces have a singular correlation matrix "
                                                 << "beyond the exactly parallel case; use the SIMULATION system algorithm";
        }
    }
    probability += ((bitCount(mask) % 2) == 0) ? term : -term;
  }
  if ((probability < -1e-12) || (probability > 1.0 + 1e-12))
    throw InternalException(HERE) << "The interval probability is outside [0, 1]: " << probability;
  return std::min(std::max(probability, 0.0), 1.0);
}

/* Probability of the intersection of the half-spaces
   {a_k^T u <= threshold_k} in the standard normal space. */
Scalar halfSpaceProbability(const Sample & unitNormals,
                            const Point & thresholds,
                            const UnsignedInteger maximumTermNumber)
{
  return intervalProbability(buildHalfSpaceGroups(unitNormals, thresholds), maximumTermNumber);
}

} /* namespace */

/* Default constructor */
ReliabilityEstimator::ReliabilityEstimator()
  : ProbabilityEstimatorImplementation()
  , solver_(RBDOAlgorithmTools::BuildDefaultInnerSolver("ReliabilityEstimator"))
  , maximumDesignPointsNumber_(ResourceMap::GetAsUnsignedInteger("ReliabilityEstimator-DefaultMaximumDesignPointsNumber"))
  , maximumCombinationNumber_(ResourceMap::GetAsUnsignedInteger("ReliabilityEstimator-DefaultMaximumCombinationNumber"))
  , betaTarget_(ResourceMap::GetAsScalar("ReliabilityEstimator-DefaultBetaTarget"))
  , sampleSize_(ResourceMap::GetAsUnsignedInteger("ReliabilityEstimator-DefaultSampleSize"))
{
  // Nothing to do
}

/* Parameter constructor */
ReliabilityEstimator::ReliabilityEstimator(const String & componentAlgorithm,
                                           const String & systemAlgorithm)
  : ReliabilityEstimator()
{
  setComponentAlgorithm(componentAlgorithm);
  setSystemAlgorithm(systemAlgorithm);
}

/* Virtual constructor */
ReliabilityEstimator * ReliabilityEstimator::clone() const
{
  return new ReliabilityEstimator(*this);
}

/* Whether the estimator provides a geometry */
Bool ReliabilityEstimator::hasGeometry() const
{
  return systemAlgorithm_ == "ANALYTICAL";
}

/* Signed reliability index of each limit-state component */
Point ReliabilityEstimator::getComponentReliabilityIndexes(const Function & predicate,
                                                           const Function & limitState,
                                                           const Distribution & distribution,
                                                           const Point & designPoint) const
{
  if (!hasGeometry())
    throw NotYetImplementedException(HERE) << "The SIMULATION system algorithm does not provide a design-point geometry; "
                                           << "use the ANALYTICAL system algorithm";
  return analyze(predicate, limitState, distribution, designPoint).componentReliabilityIndexes;
}

/* Standard-space design points, grouped by limit-state component */
Sample ReliabilityEstimator::getStandardDesignPoints(const Function & predicate,
                                                     const Function & limitState,
                                                     const Distribution & distribution,
                                                     const Point & designPoint) const
{
  if (!hasGeometry())
    throw NotYetImplementedException(HERE) << "The SIMULATION system algorithm does not provide a design-point geometry; "
                                           << "use the ANALYTICAL system algorithm";
  return analyze(predicate, limitState, distribution, designPoint).standardDesignPoints;
}

/* Number of standard-space design points of each limit-state component */
Indices ReliabilityEstimator::getDesignPointsNumber(const Function & predicate,
                                                    const Function & limitState,
                                                    const Distribution & distribution,
                                                    const Point & designPoint) const
{
  if (!hasGeometry())
    throw NotYetImplementedException(HERE) << "The SIMULATION system algorithm does not provide a design-point geometry; "
                                           << "use the ANALYTICAL system algorithm";
  return analyze(predicate, limitState, distribution, designPoint).designPointsNumber;
}

/* Bounds of the probability of the linearized event */
Interval ReliabilityEstimator::getProbabilityBounds(const Function & predicate,
                                                    const Function & limitState,
                                                    const Distribution & distribution,
                                                    const Point & designPoint) const
{
  if (!hasGeometry())
    throw NotYetImplementedException(HERE) << "The SIMULATION system algorithm does not provide probability bounds; "
                                           << "use the ANALYTICAL system algorithm";
  const Analysis analysis(analyze(predicate, limitState, distribution, designPoint));
  return Interval(Point(1, analysis.safeProbabilityLowerBound),
                  Point(1, analysis.safeProbabilityUpperBound));
}

/* Component algorithm accessor */
void ReliabilityEstimator::setComponentAlgorithm(const String & algorithm)
{
  if ((algorithm != "FORM") && (algorithm != "MULTIFORM"))
    throw InvalidArgumentException(HERE) << "The component algorithm must be FORM or MULTIFORM, got " << algorithm;
  componentAlgorithm_ = algorithm;
}

String ReliabilityEstimator::getComponentAlgorithm() const
{
  return componentAlgorithm_;
}

/* System algorithm accessor */
void ReliabilityEstimator::setSystemAlgorithm(const String & algorithm)
{
  if ((algorithm != "ANALYTICAL") && (algorithm != "SIMULATION"))
    throw InvalidArgumentException(HERE) << "The system algorithm must be ANALYTICAL or SIMULATION, got " << algorithm;
  systemAlgorithm_ = algorithm;
}

String ReliabilityEstimator::getSystemAlgorithm() const
{
  return systemAlgorithm_;
}

/* Nearest-point solver accessor */
void ReliabilityEstimator::setSolver(const OptimizationAlgorithm & solver)
{
  solver_ = solver;
}

OptimizationAlgorithm ReliabilityEstimator::getSolver() const
{
  return solver_;
}

/* Maximum number of design points of one component */
void ReliabilityEstimator::setMaximumDesignPointsNumber(const UnsignedInteger number)
{
  if (number == 0)
    throw InvalidArgumentException(HERE) << "The maximum number of design points must be positive";
  maximumDesignPointsNumber_ = number;
}

UnsignedInteger ReliabilityEstimator::getMaximumDesignPointsNumber() const
{
  return maximumDesignPointsNumber_;
}

/* Maximum number of atoms of the exact combination */
void ReliabilityEstimator::setMaximumCombinationNumber(const UnsignedInteger number)
{
  if (number == 0)
    throw InvalidArgumentException(HERE) << "The maximum number of combination atoms must be positive";
  maximumCombinationNumber_ = number;
}

UnsignedInteger ReliabilityEstimator::getMaximumCombinationNumber() const
{
  return maximumCombinationNumber_;
}

/* Nominal reliability index used to seed the inner analyses */
void ReliabilityEstimator::setBetaTarget(const Scalar betaTarget)
{
  if (!(betaTarget > 0.0) || !std::isfinite(betaTarget))
    throw InvalidArgumentException(HERE) << "The nominal reliability index must be finite and positive, got " << betaTarget;
  betaTarget_ = betaTarget;
}

Scalar ReliabilityEstimator::getBetaTarget() const
{
  return betaTarget_;
}

/* Sample size of the simulation system algorithm */
void ReliabilityEstimator::setSampleSize(const UnsignedInteger size)
{
  if (size == 0)
    throw InvalidArgumentException(HERE) << "The simulation sample size must be positive";
  sampleSize_ = size;
}

UnsignedInteger ReliabilityEstimator::getSampleSize() const
{
  return sampleSize_;
}

/* Design points of one limit-state component */
ReliabilityEstimator::ComponentDesignPointCollection
ReliabilityEstimator::solveComponent(const Function & standardFunction) const
{
  const UnsignedInteger maximumNumber = (componentAlgorithm_ == "FORM") ? 1 : maximumDesignPointsNumber_;
  const Scalar gamma = ResourceMap::GetAsScalar("MultiFORM-DefaultGamma");
  const Scalar delta = ResourceMap::GetAsScalar("MultiFORM-DefaultDelta");
  const Scalar gammaSquare = gamma * gamma;
  const Scalar deltaSquare = delta * delta;
  const Scalar separationBound = std::acos(1.0 - 0.5 * gammaSquare);
  const Scalar tolerance = ResourceMap::GetAsScalar("ReliabilityEstimator-DesignPointTolerance");

  // The sign of the index is a property of the original component, not of the
  // bulged restart function whose origin value may be shifted by a bulge.
  const Point origin(standardFunction.getInputDimension());
  const Scalar originValue(standardFunction(origin)[0]);
  if (!std::isfinite(originValue))
    throw InternalException(HERE) << "The limit-state function returned a non-finite value at the origin of the standard space: "
                                  << originValue;
  ComponentDesignPointCollection result;
  Collection<Function> terms(1, standardFunction);
  Point coefficients(1, 1.0);
  Function currentFunction(standardFunction);
  Point warmStart;
  for (UnsignedInteger k = 0; k < maximumNumber; ++k)
  {
    RBDOAlgorithmTools::InnerReport report;
    try
    {
      report = RBDOAlgorithmTools::SolveRIA(currentFunction, betaTarget_, solver_, warmStart);
    }
    catch (const Exception &)
    {
      // A failed restart only ends the enumeration: the design points already
      // accepted remain valid first-order mechanisms.
      if (k == 0)
        throw;
      break;
    }
    const Point standardPoint(report.standardDesignPoint);
    const Scalar distance(standardPoint.norm());
    if (!(distance > tolerance))
      throw InvalidArgumentException(HERE) << "The design point of the limit state is the origin of the standard space: "
                                           << "the first-order linearization is undefined at this design point";
    if (std::abs(std::abs(report.beta) - distance) > 1e-6 * std::max(1.0, distance))
      throw InternalException(HERE) << "The reliability analysis returned an inconsistent index " << report.beta
                                    << " for a design point at distance " << distance;
    // A restart that returns a design point too close to an accepted one
    // brings no new mechanism: stop the enumeration.
    Bool separated = true;
    for (UnsignedInteger i = 0; i < result.getSize(); ++i)
    {
      const Scalar previousDistance = std::abs(result[i].reliabilityIndex);
      const Scalar rho = standardPoint.dot(result[i].standardPoint) / (distance * previousDistance);
      if (std::acos(std::max(-1.0, std::min(1.0, rho))) <= separationBound)
      {
        separated = false;
        break;
      }
    }
    if (!separated)
      break;
    ComponentDesignPoint designPoint;
    designPoint.standardPoint = standardPoint;
    designPoint.reliabilityIndex = (originValue >= 0.0) ? distance : -distance;
    result.add(designPoint);
    if (result.getSize() >= maximumNumber)
      break;
    // Push the next nearest-point search away from the accepted design point.
    if (gammaSquare <= deltaSquare)
      break;
    const Scalar radius = gamma * distance;
    if (!(radius > 0.0))
      break;
    const Matrix gradient(standardFunction.gradient(standardPoint));
    const Scalar gradientNorm = std::sqrt(gradient.computeGram()(0, 0));
    if (!(gradientNorm > 0.0) || !std::isfinite(gradientNorm))
      break;
    const Scalar height = delta * gammaSquare * gradientNorm
                          / ((gammaSquare - deltaSquare) * (gammaSquare - deltaSquare) * distance);
    if (!(height > 0.0) || !std::isfinite(height))
      break;
    terms.add(Function(new BulgeEvaluation(standardPoint, radius, height),
                       new BulgeGradient(standardPoint, radius, height),
                       new BulgeHessian(standardPoint, radius, height)));
    coefficients.add(1.0);
    currentFunction = LinearCombinationFunction(terms, coefficients);
    warmStart = standardPoint * (-1.0);
  }
  return result;
}

/* Analytical system probability over the linearized components */
void ReliabilityEstimator::combineSystem(const Function & predicate,
                                         const UnsignedInteger componentNumber,
                                         const Collection<ComponentDesignPointCollection> & componentDesignPoints,
                                         Analysis & analysis) const
{
  // One unit normal and one threshold per linearized half-space
  Collection<ComponentDesignPoint> halfSpaces;
  Indices componentStart(componentNumber);
  analysis.designPointsNumber = Indices(componentNumber);
  for (UnsignedInteger j = 0; j < componentNumber; ++j)
  {
    componentStart[j] = halfSpaces.getSize();
    analysis.designPointsNumber[j] = componentDesignPoints[j].getSize();
    for (UnsignedInteger k = 0; k < componentDesignPoints[j].getSize(); ++k)
      halfSpaces.add(componentDesignPoints[j][k]);
  }
  const UnsignedInteger halfSpaceNumber = halfSpaces.getSize();
  const UnsignedInteger dimension = halfSpaces[0].standardPoint.getDimension();
  Sample unitNormals(halfSpaceNumber, dimension);
  Point thresholds(halfSpaceNumber);
  for (UnsignedInteger k = 0; k < halfSpaceNumber; ++k)
  {
    const Scalar index = halfSpaces[k].reliabilityIndex;
    thresholds[k] = index;
    for (UnsignedInteger i = 0; i < dimension; ++i)
      unitNormals(k, i) = halfSpaces[k].standardPoint[i] / index;
  }
  // Component safe probabilities from the linearized sub-events
  analysis.componentReliabilityIndexes = Point(componentNumber);
  Point componentProbabilities(componentNumber);
  for (UnsignedInteger j = 0; j < componentNumber; ++j)
  {
    const UnsignedInteger start = componentStart[j];
    const UnsignedInteger size = componentDesignPoints[j].getSize();
    Sample componentNormals(size, dimension);
    Point componentThresholds(size);
    for (UnsignedInteger i = 0; i < size; ++i)
    {
      componentThresholds[i] = thresholds[start + i];
      for (UnsignedInteger l = 0; l < dimension; ++l)
        componentNormals(i, l) = unitNormals(start + i, l);
    }
    const Scalar safeProbability = halfSpaceProbability(componentNormals, componentThresholds,
                                                        maximumCombinationNumber_);
    componentProbabilities[j] = safeProbability;
    // Keep the reported index finite when the first-order probability
    // saturates: the same guard as the chance measures.
    const Scalar clippedProbability = std::min(std::max(safeProbability, SpecFunc::Precision),
                                               1.0 - SpecFunc::Precision);
    analysis.componentReliabilityIndexes[j] = DistFunc::qNormal(clippedProbability);
  }

  // Exact combination over the 2^N atoms of the half-spaces, or bounds over
  // the 2^m atoms of the components when the exact sum is too large.
  const UnsignedInteger halfSpaceAtomNumber = truthTableSize(halfSpaceNumber);
  const UnsignedInteger componentAtomNumber = truthTableSize(componentNumber);
  if ((halfSpaceAtomNumber > 0) && (halfSpaceAtomNumber <= maximumCombinationNumber_))
  {
    // Compensated sum: the atoms form a partition of the event, so the sum is
    // a probability even when it contains many small terms.
    Scalar probability = 0.0;
    Scalar compensation = 0.0;
    Sample atomNormals(halfSpaceNumber, dimension);
    Point atomThresholds(halfSpaceNumber);
    Point indicators(componentNumber);
    for (UnsignedInteger pattern = 0; pattern < halfSpaceAtomNumber; ++pattern)
    {
      for (UnsignedInteger k = 0; k < halfSpaceNumber; ++k)
      {
        const Bool safe = ((pattern >> k) & 1) != 0;
        atomThresholds[k] = safe ? thresholds[k] : -thresholds[k];
        for (UnsignedInteger l = 0; l < dimension; ++l)
          atomNormals(k, l) = (safe ? 1.0 : -1.0) * unitNormals(k, l);
      }
      for (UnsignedInteger j = 0; j < componentNumber; ++j)
      {
        const UnsignedInteger begin = componentStart[j];
        const UnsignedInteger end = begin + analysis.designPointsNumber[j];
        Bool safe = true;
        for (UnsignedInteger k = begin; k < end; ++k)
          if (!((pattern >> k) & 1))
          {
            safe = false;
            break;
          }
        indicators[j] = safe ? 1.0 : 0.0;
      }
      if (ProbabilityEstimatorTools::evaluatePredicate(predicate, indicators) == 1.0)
      {
        const Scalar atomProbability = halfSpaceProbability(atomNormals, atomThresholds, maximumCombinationNumber_);
        const Scalar correctedAtom = atomProbability - compensation;
        const Scalar updatedProbability = probability + correctedAtom;
        compensation = (updatedProbability - probability) - correctedAtom;
        probability = updatedProbability;
      }
    }
    analysis.safeProbability = probability;
    analysis.safeProbabilityLowerBound = probability;
    analysis.safeProbabilityUpperBound = probability;
    analysis.exact = true;
    analysis.estimationError = std::numeric_limits<Scalar>::quiet_NaN();
    return;
  }
  if ((componentAtomNumber == 0) || (componentAtomNumber > maximumCombinationNumber_))
    throw NotYetImplementedException(HERE) << "The system has " << halfSpaceNumber << " linearized half-spaces and "
                                     << componentNumber << " components: the exact combination needs "
                                     << halfSpaceAtomNumber << " atoms and the bounds need "
                                     << componentAtomNumber << " atoms, both above the maximum of "
                                     << maximumCombinationNumber_ << "; use the SIMULATION system algorithm "
                                     << "or the ExperimentEstimator";
  Scalar lowerBound = 0.0;
  Scalar upperBound = 0.0;
  Point indicators(componentNumber);
  for (UnsignedInteger pattern = 0; pattern < componentAtomNumber; ++pattern)
  {
    Scalar atomLowerBound = 0.0;
    Scalar atomUpperBound = 1.0;
    for (UnsignedInteger j = 0; j < componentNumber; ++j)
    {
      const Scalar safeProbability = componentProbabilities[j];
      const Scalar literalProbability = ((pattern >> j) & 1) ? safeProbability : 1.0 - safeProbability;
      // Bonferroni lower bound of the intersection, smallest literal as upper
      atomLowerBound += 1.0 - literalProbability;
      atomUpperBound = std::min(atomUpperBound, literalProbability);
      indicators[j] = ((pattern >> j) & 1) ? 1.0 : 0.0;
    }
    if (ProbabilityEstimatorTools::evaluatePredicate(predicate, indicators) == 1.0)
    {
      lowerBound += std::max(0.0, 1.0 - atomLowerBound);
      upperBound += atomUpperBound;
    }
  }
  analysis.safeProbabilityLowerBound = std::max(0.0, std::min(1.0, lowerBound));
  analysis.safeProbabilityUpperBound = std::max(0.0, std::min(1.0, upperBound));
  if (analysis.safeProbabilityLowerBound > analysis.safeProbabilityUpperBound)
    throw InternalException(HERE) << "The system bounds are inconsistent: "
                                  << analysis.safeProbabilityLowerBound << " > "
                                  << analysis.safeProbabilityUpperBound;
  analysis.safeProbability = 0.5 * (analysis.safeProbabilityLowerBound + analysis.safeProbabilityUpperBound);
  analysis.exact = false;
  analysis.estimationError = 0.5 * (analysis.safeProbabilityUpperBound - analysis.safeProbabilityLowerBound);
}

/* Simulation system probability */
void ReliabilityEstimator::simulateSystem(const Function & predicate,
                                          const Function & limitState,
                                          const Distribution & distribution,
                                          const Point & designPoint,
                                          Analysis & analysis) const
{
  const Sample thetaSample(distribution.getSample(sampleSize_));
  UnsignedInteger safeNumber = 0;
  for (UnsignedInteger i = 0; i < sampleSize_; ++i)
  {
    const Point limitStateValue(ProbabilityEstimatorTools::evaluateLimitState(limitState, designPoint, thetaSample[i]));
    const Point indicators(ProbabilityEstimatorTools::buildIndicators(limitStateValue));
    if (ProbabilityEstimatorTools::evaluatePredicate(predicate, indicators) == 1.0)
      ++safeNumber;
  }
  const Scalar probability = 1.0 * safeNumber / (1.0 * sampleSize_);
  analysis.safeProbability = probability;
  analysis.safeProbabilityLowerBound = probability;
  analysis.safeProbabilityUpperBound = probability;
  analysis.exact = false;
  analysis.estimationError = std::sqrt(probability * (1.0 - probability) / (1.0 * sampleSize_));
}

/* Run the analysis at the design point */
ReliabilityEstimator::Analysis ReliabilityEstimator::analyze(const Function & predicate,
                                                             const Function & limitState,
                                                             const Distribution & distribution,
                                                             const Point & designPoint) const
{
  ProbabilityEstimatorTools::checkArguments(predicate, limitState, distribution, designPoint);
  Analysis analysis;
  const UnsignedInteger componentNumber = limitState.getOutputDimension();
  if (systemAlgorithm_ == "SIMULATION")
  {
    simulateSystem(predicate, limitState, distribution, designPoint, analysis);
    return analysis;
  }
  // The multinormal orthant probability is valid only in a normal standard
  // space: reject any other standard distribution instead of silently using
  // the normal combination there.
  const UnsignedInteger dimension = distribution.getDimension();
  const Distribution standardDistribution(distribution.getStandardDistribution());
  if (!(standardDistribution.getDimension() == dimension) || !(standardDistribution == Normal(dimension)))
    throw InvalidArgumentException(HERE) << "The analytical system algorithm requires a normal standard space, but "
                                         << distribution.getImplementation()->getClassName()
                                         << " has standard distribution " << standardDistribution
                                         << "; use the SIMULATION system algorithm or the ExperimentEstimator";
  // The frozen marginal is built from the marginal limit state: taking the
  // marginal of the composed frozen function would go through the generic
  // Function::getMarginal path, which cannot marginalize the NoHessian
  // placeholder of the frozen function.
  const Function inverseTransform(distribution.getInverseIsoProbabilisticTransformation());
  Collection<ComponentDesignPointCollection> componentDesignPoints(componentNumber);
  for (UnsignedInteger j = 0; j < componentNumber; ++j)
  {
    const Function componentLimitState(limitState.getMarginal(j));
    const Function frozenComponent(ProbabilityEstimatorTools::buildFrozenLimitState(componentLimitState, designPoint));
    const Function componentFunction(ComposedFunction(frozenComponent, inverseTransform));
    componentDesignPoints[j] = solveComponent(componentFunction);
    if (componentDesignPoints[j].getSize() == 0)
      throw InternalException(HERE) << "The reliability analysis of component " << j << " returned no design point";
  }
  combineSystem(predicate, componentNumber, componentDesignPoints, analysis);
  Sample designPoints(0, dimension);
  for (UnsignedInteger j = 0; j < componentNumber; ++j)
    for (UnsignedInteger k = 0; k < componentDesignPoints[j].getSize(); ++k)
      designPoints.add(componentDesignPoints[j][k].standardPoint);
  analysis.standardDesignPoints = designPoints;
  return analysis;
}

/* Probability implementation */
Scalar ReliabilityEstimator::computeSafeProbability(const Function & predicate,
                                                    const Function & limitState,
                                                    const Distribution & distribution,
                                                    const Point & designPoint) const
{
  return analyze(predicate, limitState, distribution, designPoint).safeProbability;
}

/* Error implementation */
Scalar ReliabilityEstimator::computeEstimationError(const Function & predicate,
                                                    const Function & limitState,
                                                    const Distribution & distribution,
                                                    const Point & designPoint) const
{
  return analyze(predicate, limitState, distribution, designPoint).estimationError;
}

/* String converter */
String ReliabilityEstimator::__repr__() const
{
  OSS oss;
  oss << "class=" << ReliabilityEstimator::GetClassName()
      << " componentAlgorithm=" << componentAlgorithm_
      << " systemAlgorithm=" << systemAlgorithm_
      << " maximumDesignPointsNumber=" << maximumDesignPointsNumber_
      << " maximumCombinationNumber=" << maximumCombinationNumber_
      << " betaTarget=" << betaTarget_
      << " sampleSize=" << sampleSize_
      << " solver=" << solver_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void ReliabilityEstimator::save(Advocate & adv) const
{
  ProbabilityEstimatorImplementation::save(adv);
  adv.saveAttribute("componentAlgorithm_", componentAlgorithm_);
  adv.saveAttribute("systemAlgorithm_", systemAlgorithm_);
  adv.saveAttribute("solver_", solver_);
  adv.saveAttribute("maximumDesignPointsNumber_", maximumDesignPointsNumber_);
  adv.saveAttribute("maximumCombinationNumber_", maximumCombinationNumber_);
  adv.saveAttribute("betaTarget_", betaTarget_);
  adv.saveAttribute("sampleSize_", sampleSize_);
}

/* Method load() reloads the object from the StorageManager */
void ReliabilityEstimator::load(Advocate & adv)
{
  ProbabilityEstimatorImplementation::load(adv);
  adv.loadAttribute("componentAlgorithm_", componentAlgorithm_);
  adv.loadAttribute("systemAlgorithm_", systemAlgorithm_);
  adv.loadAttribute("solver_", solver_);
  adv.loadAttribute("maximumDesignPointsNumber_", maximumDesignPointsNumber_);
  adv.loadAttribute("maximumCombinationNumber_", maximumCombinationNumber_);
  adv.loadAttribute("betaTarget_", betaTarget_);
  adv.loadAttribute("sampleSize_", sampleSize_);
}

} /* namespace OTROBOPT */
