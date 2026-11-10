//                                               -*- C++ -*-
/**
 *  @brief Canonicalization of robust optimization problems
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
#include "otrobopt/RobustOptimizationProblemCanonicalizer.hxx"

#include <algorithm>
#include <cmath>

#include <openturns/AggregatedFunction.hxx>
#include <openturns/ComposedFunction.hxx>
#include <openturns/Exception.hxx>
#include <openturns/LinearFunction.hxx>
#include <openturns/Matrix.hxx>
#include <openturns/Normal.hxx>
#include <openturns/ParametricFunction.hxx>

#include "otrobopt/AggregatedMeasure.hxx"
#include "otrobopt/ParameterAsInputFunction.hxx"
#include "otrobopt/ParametricDistributionTransformation.hxx"

using namespace OT;

namespace OTROBOPT
{

namespace
{

Distribution cloneDistribution(const Distribution & distribution)
{
  return Distribution(distribution.getImplementation()->clone());
}

Bool isBounded(const Interval & interval)
{
  if (interval.getDimension() == 0)
    return false;
  const Interval::BoolCollection finiteLower(interval.getFiniteLowerBound());
  const Interval::BoolCollection finiteUpper(interval.getFiniteUpperBound());
  for (UnsignedInteger i = 0; i < interval.getDimension(); ++i)
    if (!finiteLower[i] || !finiteUpper[i])
      return false;
  return true;
}

Interval filterInterval(const Interval & interval, const Indices & indices)
{
  const UnsignedInteger dimension = indices.getSize();
  const Point lowerBound(interval.getLowerBound());
  const Point upperBound(interval.getUpperBound());
  const Interval::BoolCollection finiteLower(interval.getFiniteLowerBound());
  const Interval::BoolCollection finiteUpper(interval.getFiniteUpperBound());
  Point filteredLower(dimension);
  Point filteredUpper(dimension);
  Interval::BoolCollection filteredFiniteLower(dimension);
  Interval::BoolCollection filteredFiniteUpper(dimension);
  for (UnsignedInteger i = 0; i < dimension; ++i)
  {
    filteredLower[i] = lowerBound[indices[i]];
    filteredUpper[i] = upperBound[indices[i]];
    filteredFiniteLower[i] = finiteLower[indices[i]];
    filteredFiniteUpper[i] = finiteUpper[indices[i]];
  }
  return Interval(filteredLower, filteredUpper, filteredFiniteLower, filteredFiniteUpper);
}

void checkActiveParameter(const Indices & active, const UnsignedInteger fullDimension)
{
  Indices sorted(active);
  std::sort(sorted.begin(), sorted.end());
  if (std::unique(sorted.begin(), sorted.end()) != sorted.end())
    throw InvalidArgumentException(HERE) << "The active parameter indices must be unique";
  for (UnsignedInteger i = 0; i < active.getSize(); ++i)
    if (active[i] >= fullDimension)
      throw InvalidArgumentException(HERE) << "The active parameter index " << active[i]
                                           << " is out of range [0, " << fullDimension << ")";
}

String policyName(const CanonicalizationOptions::WorstCasePolicy policy)
{
  switch (policy)
  {
    case CanonicalizationOptions::AUTO:
      return "AUTO";
    case CanonicalizationOptions::STANDARD:
      return "STANDARD";
    case CanonicalizationOptions::NORMAL:
      return "NORMAL";
    case CanonicalizationOptions::USER_REFERENCE:
      return "USER_REFERENCE";
    case CanonicalizationOptions::USER_DOMAIN:
      return "USER_DOMAIN";
  }
  return "UNKNOWN";
}

} /* anonymous namespace */

/* Default constructor */
CanonicalizationOptions::CanonicalizationOptions()
  : reference()
  , parameterBounds()
  , parameterStart()
  , activeParameter()
  , optimizeParameter(true)
  , worstCasePolicy(AUTO)
  , worstCaseDomain()
{
  // Nothing to do
}

/* Reference distribution accessors */
void CanonicalizationOptions::setReference(const Distribution & reference)
{
  if (reference.getDimension() == 0)
  {
    clearReference();
    return;
  }
  this->reference = cloneDistribution(reference);
  hasReference_ = true;
}

Distribution CanonicalizationOptions::getReference() const
{
  return cloneDistribution(reference);
}

void CanonicalizationOptions::clearReference()
{
  reference = Distribution();
  hasReference_ = false;
}

Bool CanonicalizationOptions::hasReference() const
{
  return hasReference_ || (reference != Distribution());
}

/* Parameter bounds accessors */
void CanonicalizationOptions::setParameterBounds(const Interval & bounds)
{
  parameterBounds = bounds;
}

Interval CanonicalizationOptions::getParameterBounds() const
{
  return parameterBounds;
}

/* Parameter start accessors */
void CanonicalizationOptions::setParameterStart(const Point & start)
{
  parameterStart = start;
}

Point CanonicalizationOptions::getParameterStart() const
{
  return parameterStart;
}

/* Active parameter accessors */
void CanonicalizationOptions::setActiveParameter(const Indices & active)
{
  activeParameter = active;
  hasActiveParameter_ = true;
}

Indices CanonicalizationOptions::getActiveParameter() const
{
  return activeParameter;
}

void CanonicalizationOptions::clearActiveParameter()
{
  activeParameter = Indices();
  hasActiveParameter_ = false;
}

Bool CanonicalizationOptions::hasActiveParameter() const
{
  return hasActiveParameter_;
}

/* Parameter optimization accessors */
void CanonicalizationOptions::setOptimizeParameter(const Bool optimizeParameter)
{
  this->optimizeParameter = optimizeParameter;
}

Bool CanonicalizationOptions::getOptimizeParameter() const
{
  return optimizeParameter;
}

/* Worst-case policy accessors */
void CanonicalizationOptions::setWorstCasePolicy(const WorstCasePolicy policy)
{
  if ((policy < AUTO) || (policy > USER_DOMAIN))
    throw InvalidArgumentException(HERE) << "Unknown worst-case policy";
  worstCasePolicy = policy;
}

CanonicalizationOptions::WorstCasePolicy CanonicalizationOptions::getWorstCasePolicy() const
{
  return worstCasePolicy;
}

/* Worst-case domain accessors */
void CanonicalizationOptions::setWorstCaseDomain(const Interval & domain)
{
  worstCaseDomain = domain;
}

Interval CanonicalizationOptions::getWorstCaseDomain() const
{
  return worstCaseDomain;
}

/* String converter */
String CanonicalizationOptions::__repr__() const
{
  OSS oss;
  oss << "class=CanonicalizationOptions"
      << " reference=" << reference
      << " parameterBounds=" << parameterBounds
      << " parameterStart=" << parameterStart
      << " activeParameter=" << activeParameter
      << " optimizeParameter=" << optimizeParameter
      << " worstCasePolicy=" << policyName(worstCasePolicy)
      << " worstCaseDomain=" << worstCaseDomain;
  return oss;
}

/* Parameter constructor */
RobustOptimizationProblemCanonicalizer::RobustOptimizationProblemCanonicalizer(
  const RobustOptimizationProblem & problem,
  const CanonicalizationOptions & options)
  : canonicalProblem_()
  , reference_()
  , startingPoint_()
{
  initialize(problem, options);
}

/* Whether the standard fast path is available for a distribution */
Bool RobustOptimizationProblemCanonicalizer::isStandardReference(const Distribution & distribution) const
{
  const UnsignedInteger dimension = distribution.getDimension();
  const Distribution standardDistribution(distribution.getStandardDistribution());
  return (standardDistribution.getDimension() == dimension)
         && (standardDistribution == Normal(dimension));
}

/* Select the reference distribution */
Distribution RobustOptimizationProblemCanonicalizer::selectReference(
  const Distribution & distribution,
  const CanonicalizationOptions & options,
  Bool & usesStandardSpace) const
{
  const UnsignedInteger dimension = distribution.getDimension();
  const Bool hasReference = options.hasReference();
  const Bool standardUsable = isStandardReference(distribution);

  switch (options.worstCasePolicy)
  {
    case CanonicalizationOptions::STANDARD:
      if (hasReference)
        throw InvalidArgumentException(HERE) << "The STANDARD worst-case policy does not accept a user reference distribution";
      if (!standardUsable)
        throw InvalidArgumentException(HERE) << "The STANDARD worst-case policy requires a parameter-free standard distribution, "
                                             << "but " << distribution.getImplementation()->getClassName()
                                             << " has standard distribution " << distribution.getStandardDistribution();
      usesStandardSpace = true;
      return cloneDistribution(distribution.getStandardDistribution());

    case CanonicalizationOptions::NORMAL:
      if (hasReference)
        throw InvalidArgumentException(HERE) << "The NORMAL worst-case policy does not accept a user reference distribution";
      usesStandardSpace = false;
      return Normal(dimension);

    case CanonicalizationOptions::USER_REFERENCE:
      if (!hasReference)
        throw InvalidArgumentException(HERE) << "The USER_REFERENCE worst-case policy requires a reference distribution";
      break;

    case CanonicalizationOptions::USER_DOMAIN:
    case CanonicalizationOptions::AUTO:
      break;

    default:
      throw InvalidArgumentException(HERE) << "Unknown worst-case policy";
  }

  if (hasReference)
  {
    const Distribution userReference(options.getReference());
    if (userReference.getDimension() != dimension)
      throw InvalidDimensionException(HERE) << "The reference dimension ("
                                            << userReference.getDimension()
                                            << ") must match the distribution dimension ("
                                            << dimension << ")";
    if (!userReference.isContinuous())
      throw InvalidArgumentException(HERE) << "The reference distribution must be continuous, got "
                                           << userReference.getImplementation()->getClassName();
    usesStandardSpace = standardUsable
                        && (userReference == distribution.getStandardDistribution());
    return userReference;
  }

  if (standardUsable)
  {
    usesStandardSpace = true;
    return cloneDistribution(distribution.getStandardDistribution());
  }

  usesStandardSpace = false;
  return Normal(dimension);
}

/* Check that a measure and its aggregate children share the same distribution */
void RobustOptimizationProblemCanonicalizer::checkMeasureDistribution(
  const MeasureEvaluation & measure,
  const Distribution & distribution,
  const String & role) const
{
  if (measure.getFunction().getInputDimension() == 0)
    return;
  if (measure.getDistribution() != distribution)
    throw InvalidArgumentException(HERE) << "The " << role
                                         << " measure distribution differs from the distribution of the other measures";
  const AggregatedMeasure * aggregate = dynamic_cast<const AggregatedMeasure *>(measure.getImplementation().get());
  if (aggregate)
  {
    const AggregatedMeasure::MeasureEvaluationCollection collection(aggregate->getCollection());
    for (UnsignedInteger i = 0; i < collection.getSize(); ++i)
      checkMeasureDistribution(collection[i], distribution, role);
  }
}

/* Build the canonical function P(x, s; xi) = f(x; T_s(xi)) */
Function RobustOptimizationProblemCanonicalizer::buildCanonicalFunction(
  const Function & function,
  const Function & transformation,
  const UnsignedInteger designDimension,
  const UnsignedInteger parameterDimension,
  const UnsignedInteger referenceDimension) const
{
  const UnsignedInteger totalInputDimension = designDimension + parameterDimension + referenceDimension;

  // pi_x(x, s, xi) = x
  Matrix selectDesign(designDimension, totalInputDimension);
  for (UnsignedInteger i = 0; i < designDimension; ++i)
    selectDesign(i, i) = 1.0;
  const LinearFunction designProjection(Point(totalInputDimension),
                                        Point(designDimension),
                                        selectDesign);

  // pi_xi_s(x, s, xi) = (xi, s), the input order of ParameterAsInputFunction(T)
  Matrix selectReferenceParameter(parameterDimension + referenceDimension, totalInputDimension);
  const UnsignedInteger referenceOffset = designDimension + parameterDimension;
  for (UnsignedInteger i = 0; i < referenceDimension; ++i)
    selectReferenceParameter(i, referenceOffset + i) = 1.0;
  for (UnsignedInteger i = 0; i < parameterDimension; ++i)
    selectReferenceParameter(referenceDimension + i, designDimension + i) = 1.0;
  const LinearFunction referenceParameterProjection(Point(totalInputDimension),
                                                    Point(parameterDimension + referenceDimension),
                                                    selectReferenceParameter);

  // T(xi, s) followed by f(x, theta)
  const ParameterAsInputFunction functionAsInput(function);
  const ParameterAsInputFunction transformationAsInput(transformation);
  const ComposedFunction transformationComposed(transformationAsInput,
                                                referenceParameterProjection);
  Collection<Function> components(2);
  components[0] = designProjection;
  components[1] = transformationComposed;
  const AggregatedFunction aggregate(components);
  const ComposedFunction composed(functionAsInput, aggregate);

  // Freeze xi as the function parameter, keep (x, s) as the input
  Indices referenceIndices(referenceDimension);
  for (UnsignedInteger i = 0; i < referenceDimension; ++i)
    referenceIndices[i] = referenceOffset + i;
  const ParametricFunction parametric(composed,
                                      referenceIndices,
                                      referenceStartingPoint(reference_));
  return parametric;
}

/* Lift a function of x to a function of (x, s) */
Function RobustOptimizationProblemCanonicalizer::liftFunction(
  const Function & function,
  const UnsignedInteger designDimension,
  const UnsignedInteger parameterDimension) const
{
  if (parameterDimension == 0)
    return function;
  const UnsignedInteger totalInputDimension = designDimension + parameterDimension;
  Matrix selectDesign(designDimension, totalInputDimension);
  for (UnsignedInteger i = 0; i < designDimension; ++i)
    selectDesign(i, i) = 1.0;
  const LinearFunction designProjection(Point(totalInputDimension),
                                        Point(designDimension),
                                        selectDesign);
  return ComposedFunction(function, designProjection);
}

/* Canonicalize a measure, recursively for aggregated measures */
MeasureEvaluation RobustOptimizationProblemCanonicalizer::canonicalizeMeasure(
  const MeasureEvaluation & measure,
  const Distribution & distribution,
  const Function & transformation,
  const UnsignedInteger designDimension,
  const UnsignedInteger parameterDimension,
  const UnsignedInteger referenceDimension,
  const Interval & optimizationDomain,
  const Bool optimizeParameter) const
{
  const AggregatedMeasure * aggregate = dynamic_cast<const AggregatedMeasure *>(measure.getImplementation().get());
  if (aggregate)
  {
    const AggregatedMeasure::MeasureEvaluationCollection collection(aggregate->getCollection());
    AggregatedMeasure::MeasureEvaluationCollection rebuilt(collection.getSize());
    for (UnsignedInteger i = 0; i < collection.getSize(); ++i)
      rebuilt[i] = canonicalizeMeasure(collection[i],
                                      distribution,
                                      transformation,
                                      designDimension,
                                      parameterDimension,
                                      referenceDimension,
                                      optimizationDomain,
                                      optimizeParameter);
    return AggregatedMeasure(rebuilt);
  }

  const Function function(measure.getFunction());
  if (function.getInputDimension() != designDimension)
    throw InvalidDimensionException(HERE) << "The measure function input dimension ("
                                          << function.getInputDimension()
                                          << ") must match the design dimension ("
                                          << designDimension << ")";
  if (function.getParameterDimension() != referenceDimension)
    throw InvalidDimensionException(HERE) << "The measure function parameter dimension ("
                                          << function.getParameterDimension()
                                          << ") must match the distribution dimension ("
                                          << referenceDimension << ")";

  if (optimizeParameter)
  {
    const Function canonicalFunction(buildCanonicalFunction(function,
                                                            transformation,
                                                            designDimension,
                                                            parameterDimension,
                                                            referenceDimension));
    return measure.rebuild(canonicalFunction, distribution, optimizationDomain);
  }
  return measure.rebuild(function, distribution, optimizationDomain);
}

/* Build a finite starting point from bounds */
Point RobustOptimizationProblemCanonicalizer::referenceStartingPoint(
  const Distribution & reference) const
{
  const UnsignedInteger dimension = reference.getDimension();
  Point startingPoint(dimension);
  try
  {
    const Point mean(reference.getMean());
    if (mean.getDimension() == dimension)
    {
      Bool finite = true;
      for (UnsignedInteger i = 0; i < dimension; ++i)
        if (!std::isfinite(mean[i]))
          finite = false;
      if (finite)
        return mean;
    }
  }
  catch (const Exception &)
  {
    // The distribution has no mean, fall back to its range
  }

  const Interval range(reference.getRange());
  const Interval::BoolCollection finiteLower(range.getFiniteLowerBound());
  const Interval::BoolCollection finiteUpper(range.getFiniteUpperBound());
  const Point lowerBound(range.getLowerBound());
  const Point upperBound(range.getUpperBound());
  for (UnsignedInteger i = 0; i < dimension; ++i)
  {
    if (finiteLower[i] && finiteUpper[i])
      startingPoint[i] = 0.5 * (lowerBound[i] + upperBound[i]);
    else if (finiteLower[i])
      startingPoint[i] = lowerBound[i] + 1.0;
    else if (finiteUpper[i])
      startingPoint[i] = upperBound[i] - 1.0;
    else
      startingPoint[i] = 0.0;
  }
  return startingPoint;
}

/* Build the canonical bounds */
Interval RobustOptimizationProblemCanonicalizer::buildBounds(
  const Interval & designBounds,
  const Interval & parameterBounds,
  const UnsignedInteger designDimension,
  const UnsignedInteger parameterDimension,
  const Bool optimizeParameter) const
{
  if (!optimizeParameter)
    return designBounds;

  if ((designBounds.getDimension() != 0) && (designBounds.getDimension() != designDimension))
    throw InvalidDimensionException(HERE) << "The design bounds dimension ("
                                          << designBounds.getDimension()
                                          << ") must match the design dimension ("
                                          << designDimension << ")";
  if ((parameterBounds.getDimension() != 0) && (parameterBounds.getDimension() != parameterDimension))
    throw InvalidDimensionException(HERE) << "The parameter bounds dimension ("
                                          << parameterBounds.getDimension()
                                          << ") must match the parameter dimension ("
                                          << parameterDimension << ")";

  if ((designBounds.getDimension() == 0) && (parameterBounds.getDimension() == 0))
    return Interval();

  const UnsignedInteger dimension = designDimension + parameterDimension;
  Point lowerBound(dimension);
  Point upperBound(dimension);
  Interval::BoolCollection finiteLower(dimension, 0);
  Interval::BoolCollection finiteUpper(dimension, 0);

  if (designBounds.getDimension() != 0)
  {
    const Point designLower(designBounds.getLowerBound());
    const Point designUpper(designBounds.getUpperBound());
    const Interval::BoolCollection designFiniteLower(designBounds.getFiniteLowerBound());
    const Interval::BoolCollection designFiniteUpper(designBounds.getFiniteUpperBound());
    for (UnsignedInteger i = 0; i < designDimension; ++i)
    {
      lowerBound[i] = designLower[i];
      upperBound[i] = designUpper[i];
      finiteLower[i] = designFiniteLower[i];
      finiteUpper[i] = designFiniteUpper[i];
    }
  }
  else
  {
    for (UnsignedInteger i = 0; i < designDimension; ++i)
    {
      lowerBound[i] = 0.0;
      upperBound[i] = 0.0;
    }
  }

  if (parameterBounds.getDimension() != 0)
  {
    const Point parameterLower(parameterBounds.getLowerBound());
    const Point parameterUpper(parameterBounds.getUpperBound());
    const Interval::BoolCollection parameterFiniteLower(parameterBounds.getFiniteLowerBound());
    const Interval::BoolCollection parameterFiniteUpper(parameterBounds.getFiniteUpperBound());
    for (UnsignedInteger i = 0; i < parameterDimension; ++i)
    {
      lowerBound[designDimension + i] = parameterLower[i];
      upperBound[designDimension + i] = parameterUpper[i];
      finiteLower[designDimension + i] = parameterFiniteLower[i];
      finiteUpper[designDimension + i] = parameterFiniteUpper[i];
    }
  }
  else
  {
    for (UnsignedInteger i = 0; i < parameterDimension; ++i)
    {
      lowerBound[designDimension + i] = 0.0;
      upperBound[designDimension + i] = 0.0;
    }
  }

  return Interval(lowerBound, upperBound, finiteLower, finiteUpper);
}

/* Build the canonical starting point */
Point RobustOptimizationProblemCanonicalizer::buildStartingPoint(
  const Interval & designBounds,
  const Point & parameterStart,
  const UnsignedInteger designDimension,
  const UnsignedInteger parameterDimension,
  const Bool optimizeParameter) const
{
  Point startingPoint(designDimension + (optimizeParameter ? parameterDimension : 0));
  const Point lowerBound(designBounds.getLowerBound());
  const Point upperBound(designBounds.getUpperBound());
  const Interval::BoolCollection finiteLower(designBounds.getFiniteLowerBound());
  const Interval::BoolCollection finiteUpper(designBounds.getFiniteUpperBound());
  for (UnsignedInteger i = 0; i < designDimension; ++i)
  {
    if ((designBounds.getDimension() != 0) && finiteLower[i] && finiteUpper[i])
      startingPoint[i] = 0.5 * (lowerBound[i] + upperBound[i]);
    else if ((designBounds.getDimension() != 0) && finiteLower[i])
      startingPoint[i] = lowerBound[i] + 1.0;
    else if ((designBounds.getDimension() != 0) && finiteUpper[i])
      startingPoint[i] = upperBound[i] - 1.0;
    else
      startingPoint[i] = 0.0;
  }
  if (optimizeParameter)
    for (UnsignedInteger i = 0; i < parameterDimension; ++i)
      startingPoint[designDimension + i] = parameterStart[i];
  return startingPoint;
}

/* Canonicalization */
void RobustOptimizationProblemCanonicalizer::initialize(
  const RobustOptimizationProblem & problem,
  const CanonicalizationOptions & options)
{
  const Bool hasRobustnessMeasure = problem.hasRobustnessMeasure();
  const Bool hasReliabilityMeasure = problem.hasReliabilityMeasure();
  if (!hasRobustnessMeasure && !hasReliabilityMeasure)
    throw InvalidArgumentException(HERE) << "The problem has no robustness nor reliability measure";

  // The canonical objective must exist
  if (!hasRobustnessMeasure && (problem.getObjective().getInputDimension() == 0))
    throw InvalidArgumentException(HERE) << "The problem has no objective";

  if ((options.worstCasePolicy == CanonicalizationOptions::USER_DOMAIN)
      && (options.worstCaseDomain.getDimension() == 0))
    throw InvalidArgumentException(HERE) << "The USER_DOMAIN worst-case policy requires a non-empty worst-case domain";

  // Distribution of the measures
  const MeasureEvaluation referenceMeasure = hasRobustnessMeasure
      ? problem.getRobustnessMeasure()
      : problem.getReliabilityMeasure();
  const Distribution distribution(referenceMeasure.getDistribution());
  if (distribution.getDimension() == 0)
    throw InvalidArgumentException(HERE) << "The measure distribution has dimension zero";
  if (!distribution.isContinuous())
    throw InvalidArgumentException(HERE) << "Only continuous measure distributions are supported, got "
                                         << distribution.getImplementation()->getClassName();

  checkMeasureDistribution(referenceMeasure,
                           distribution,
                           hasRobustnessMeasure ? "objective" : "constraint");
  if (hasRobustnessMeasure && hasReliabilityMeasure)
    checkMeasureDistribution(problem.getReliabilityMeasure(), distribution, "constraint");

  const UnsignedInteger designDimension = referenceMeasure.getFunction().getInputDimension();
  const UnsignedInteger distributionDimension = distribution.getDimension();
  const UnsignedInteger fullParameterDimension = distribution.getParameterDimension();
  activeParameter_ = options.hasActiveParameter()
                     ? options.getActiveParameter()
                     : ParametricDistributionTransformation::GetDefaultActiveParameter(distribution);
  checkActiveParameter(activeParameter_, fullParameterDimension);
  const UnsignedInteger parameterDimension = activeParameter_.getSize();
  optimizeParameter_ = options.optimizeParameter && (parameterDimension > 0);
  if (designDimension == 0)
    throw InvalidArgumentException(HERE) << "The measure function has input dimension zero";

  // Check the other function dimensions
  if (hasReliabilityMeasure)
  {
    const Function constraintFunction(problem.getReliabilityMeasure().getFunction());
    if (constraintFunction.getInputDimension() != designDimension)
      throw InvalidDimensionException(HERE) << "The reliability measure function input dimension ("
                                            << constraintFunction.getInputDimension()
                                            << ") must match the objective measure function input dimension ("
                                            << designDimension << ")";
  }
  else
  {
    const Function constraintFunction(problem.getInequalityConstraint());
    if ((constraintFunction.getInputDimension() != 0)
        && (constraintFunction.getInputDimension() != designDimension))
      throw InvalidDimensionException(HERE) << "The inequality constraint input dimension ("
                                            << constraintFunction.getInputDimension()
                                            << ") must match the design dimension ("
                                            << designDimension << ")";
  }

  if (hasRobustnessMeasure)
  {
    const Function objectiveFunction(problem.getRobustnessMeasure().getFunction());
    if (objectiveFunction.getOutputDimension() != 1)
      throw InvalidDimensionException(HERE) << "The robust objective must be scalar, got output dimension "
                                            << objectiveFunction.getOutputDimension();
  }
  else
  {
    const Function objectiveFunction(problem.getObjective());
    if (objectiveFunction.getOutputDimension() != 1)
      throw InvalidDimensionException(HERE) << "The robust objective must be scalar, got output dimension "
                                            << objectiveFunction.getOutputDimension();
    if (objectiveFunction.getInputDimension() != designDimension)
      throw InvalidDimensionException(HERE) << "The objective input dimension ("
                                            << objectiveFunction.getInputDimension()
                                            << ") must match the design dimension ("
                                            << designDimension << ")";
  }

  if (problem.hasEqualityConstraint()
      && (problem.getEqualityConstraint().getInputDimension() != designDimension))
    throw InvalidDimensionException(HERE) << "The equality constraint input dimension ("
                                          << problem.getEqualityConstraint().getInputDimension()
                                          << ") must match the design dimension ("
                                          << designDimension << ")";

  // Distribution parameter start, expressed in active coordinates when its
  // dimension matches the active parameter, or in full coordinates as a
  // compatibility fallback.
  Point fullParameterStart(distribution.getParameter());
  Point parameterStart(parameterDimension);
  if (options.parameterStart.getDimension() != 0)
  {
    if (options.parameterStart.getDimension() == parameterDimension)
    {
      for (UnsignedInteger i = 0; i < parameterDimension; ++i)
      {
        parameterStart[i] = options.parameterStart[i];
        fullParameterStart[activeParameter_[i]] = options.parameterStart[i];
      }
    }
    else if ((parameterDimension != fullParameterDimension)
             && (options.parameterStart.getDimension() == fullParameterDimension))
    {
      fullParameterStart = options.parameterStart;
      for (UnsignedInteger i = 0; i < parameterDimension; ++i)
        parameterStart[i] = fullParameterStart[activeParameter_[i]];
    }
    else
      throw InvalidDimensionException(HERE) << "The parameter start dimension ("
                                            << options.parameterStart.getDimension()
                                            << ") must match the active parameter dimension ("
                                            << parameterDimension << ") or the full parameter dimension ("
                                            << fullParameterDimension << ")";
  }
  else
  {
    for (UnsignedInteger i = 0; i < parameterDimension; ++i)
      parameterStart[i] = fullParameterStart[activeParameter_[i]];
  }

  // Parameter bounds are expressed in active coordinates, with the same
  // full-coordinate fallback as the starting point.
  Interval parameterBounds;
  if (options.parameterBounds.getDimension() != 0)
  {
    if (options.parameterBounds.getDimension() == parameterDimension)
      parameterBounds = options.parameterBounds;
    else if ((parameterDimension != fullParameterDimension)
             && (options.parameterBounds.getDimension() == fullParameterDimension))
      parameterBounds = filterInterval(options.parameterBounds, activeParameter_);
    else
      throw InvalidDimensionException(HERE) << "The parameter bounds dimension ("
                                            << options.parameterBounds.getDimension()
                                            << ") must match the active parameter dimension ("
                                            << parameterDimension << ") or the full parameter dimension ("
                                            << fullParameterDimension << ")";
  }

  // The worst-case domain is expressed in the reference coordinates
  const UnsignedInteger referenceDimension = distributionDimension;
  if ((options.worstCaseDomain.getDimension() != 0)
      && (options.worstCaseDomain.getDimension() != referenceDimension))
    throw InvalidDimensionException(HERE) << "The worst-case domain dimension ("
                                          << options.worstCaseDomain.getDimension()
                                          << ") must match the reference dimension ("
                                          << referenceDimension << ")";

  const Bool requiresOptimizationDomain =
    (hasRobustnessMeasure && problem.getRobustnessMeasure().requiresOptimizationDomain())
    || (hasReliabilityMeasure && problem.getReliabilityMeasure().requiresOptimizationDomain());

  Interval optimizationDomain;
  MeasureEvaluation canonicalRobustness;
  MeasureEvaluation canonicalReliability;
  Function canonicalObjective;
  Function canonicalInequality;
  Function canonicalEquality;

  if (!optimizeParameter_)
  {
    // Freeze the distribution parameter and keep the design vector only
    reference_ = distribution;
    if (fullParameterDimension > 0)
      reference_.setParameter(fullParameterStart);
    usesStandardSpace_ = false;

    if (requiresOptimizationDomain
        && (options.worstCaseDomain.getDimension() != 0))
      optimizationDomain = options.worstCaseDomain;

    if (hasRobustnessMeasure)
      canonicalRobustness = canonicalizeMeasure(problem.getRobustnessMeasure(),
                                                reference_,
                                                Function(),
                                                designDimension,
                                                parameterDimension,
                                                referenceDimension,
                                                optimizationDomain,
                                                false);
    else
      canonicalObjective = problem.getObjective();

    if (hasReliabilityMeasure)
      canonicalReliability = canonicalizeMeasure(problem.getReliabilityMeasure(),
                                                 reference_,
                                                 Function(),
                                                 designDimension,
                                                 parameterDimension,
                                                 referenceDimension,
                                                 optimizationDomain,
                                                 false);
    else
      canonicalInequality = problem.getInequalityConstraint();

    if (problem.hasEqualityConstraint())
      canonicalEquality = problem.getEqualityConstraint();
  }
  else
  {
    reference_ = selectReference(distribution, options, usesStandardSpace_);

    if (requiresOptimizationDomain)
    {
      if (options.worstCaseDomain.getDimension() != 0)
        optimizationDomain = options.worstCaseDomain;
      else if (!usesStandardSpace_ && !isBounded(reference_.getRange()))
        throw InvalidArgumentException(HERE) << "The worst-case measure requires an explicit optimization domain "
                                             << "when the reference distribution is not the standard fast path "
                                             << "and its support is unbounded. Policy: "
                                             << policyName(options.worstCasePolicy);
    }

    ParametricDistributionTransformation transformation(distribution, reference_);
    transformation.setActiveParameter(activeParameter_);
    if (parameterDimension > 0)
      transformation.setParameter(parameterStart);
    const Function transformationFunction(transformation);

    if (hasRobustnessMeasure)
    {
      canonicalRobustness = canonicalizeMeasure(problem.getRobustnessMeasure(),
                                                reference_,
                                                transformationFunction,
                                                designDimension,
                                                parameterDimension,
                                                referenceDimension,
                                                optimizationDomain,
                                                true);
    }
    else
    {
      canonicalObjective = liftFunction(problem.getObjective(),
                                        designDimension,
                                        parameterDimension);
    }

    if (hasReliabilityMeasure)
    {
      canonicalReliability = canonicalizeMeasure(problem.getReliabilityMeasure(),
                                                 reference_,
                                                 transformationFunction,
                                                 designDimension,
                                                 parameterDimension,
                                                 referenceDimension,
                                                 optimizationDomain,
                                                 true);
    }
    else
    {
      const Function inequalityConstraint(problem.getInequalityConstraint());
      if (inequalityConstraint.getInputDimension() != 0)
        canonicalInequality = liftFunction(inequalityConstraint,
                                           designDimension,
                                           parameterDimension);
    }

    if (problem.hasEqualityConstraint())
      canonicalEquality = liftFunction(problem.getEqualityConstraint(),
                                       designDimension,
                                       parameterDimension);
  }

  // Assemble the canonical problem
  if (hasRobustnessMeasure)
    canonicalProblem_.setRobustnessMeasure(canonicalRobustness);
  else
    canonicalProblem_.setObjective(canonicalObjective);
  if (hasReliabilityMeasure)
    canonicalProblem_.setReliabilityMeasure(canonicalReliability);
  else if (canonicalInequality.getInputDimension() != 0)
    canonicalProblem_.setInequalityConstraint(canonicalInequality);
  if (canonicalEquality.getInputDimension() != 0)
    canonicalProblem_.setEqualityConstraint(canonicalEquality);

  canonicalProblem_.setBounds(buildBounds(problem.getBounds(),
                                          parameterBounds,
                                          designDimension,
                                          parameterDimension,
                                          optimizeParameter_));

  startingPoint_ = buildStartingPoint(problem.getBounds(),
                                      parameterStart,
                                      designDimension,
                                      parameterDimension,
                                      optimizeParameter_);

  // Preserve the original objective direction
  if (problem.getObjective().getOutputDimension() > 0)
    canonicalProblem_.setMinimization(problem.isMinimization(0), 0);

  // Preserve the design variable types and make the parameter continuous
  Indices variableTypes(problem.getVariablesType());
  if (variableTypes.getSize() != designDimension)
    variableTypes = Indices(designDimension, OptimizationProblemImplementation::CONTINUOUS);
  if (optimizeParameter_)
    for (UnsignedInteger i = 0; i < parameterDimension; ++i)
      variableTypes.add(OptimizationProblemImplementation::CONTINUOUS);
  canonicalProblem_.setVariablesType(variableTypes);
}

/* Canonical problem accessor */
RobustOptimizationProblem RobustOptimizationProblemCanonicalizer::getCanonicalProblem() const
{
  return canonicalProblem_;
}

/* Reference distribution accessor */
Distribution RobustOptimizationProblemCanonicalizer::getReferenceDistribution() const
{
  return cloneDistribution(reference_);
}

/* Canonical objective function accessor */
Function RobustOptimizationProblemCanonicalizer::getCanonicalObjectiveFunction() const
{
  return canonicalProblem_.getObjective();
}

/* Canonical inequality constraint function accessor */
Function RobustOptimizationProblemCanonicalizer::getCanonicalConstraintFunction() const
{
  return canonicalProblem_.getInequalityConstraint();
}

/* Canonical starting point accessor */
Point RobustOptimizationProblemCanonicalizer::getCanonicalStartingPoint() const
{
  return startingPoint_;
}

/* Whether the distribution parameter is optimized */
Bool RobustOptimizationProblemCanonicalizer::getOptimizeParameter() const
{
  return optimizeParameter_;
}

/* Active distribution parameter indices used by the canonical problem */
Indices RobustOptimizationProblemCanonicalizer::getActiveParameter() const
{
  return activeParameter_;
}

/* Whether the standard fast path is used */
Bool RobustOptimizationProblemCanonicalizer::usesStandardSpace() const
{
  return usesStandardSpace_;
}

/* String converter */
String RobustOptimizationProblemCanonicalizer::__repr__() const
{
  OSS oss;
  oss << "class=RobustOptimizationProblemCanonicalizer"
      << " reference=" << reference_
      << " activeParameter=" << activeParameter_
      << " optimizeParameter=" << optimizeParameter_
      << " usesStandardSpace=" << usesStandardSpace_
      << " startingPoint=" << startingPoint_
      << " canonicalProblem=" << canonicalProblem_;
  return oss;
}

} /* namespace OTROBOPT */
