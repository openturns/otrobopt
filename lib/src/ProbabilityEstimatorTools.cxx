//                                               -*- C++ -*-
/**
 *  @brief Internal evaluation helpers for the probability estimators
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
#include "otrobopt/ProbabilityEstimatorTools.hxx"
#include <openturns/Description.hxx>
#include <openturns/EvaluationImplementation.hxx>
#include <openturns/Exception.hxx>
#include <openturns/GradientImplementation.hxx>
#include <openturns/NoHessian.hxx>
#include <openturns/ParametricEvaluation.hxx>

using namespace OT;

namespace OTROBOPT
{
namespace
{

/* Frozen limit-state function: theta -> G(designPoint, theta) */
class FrozenLimitStateEvaluation
  : public EvaluationImplementation
{
public:
  FrozenLimitStateEvaluation(const Function & function,
                             const Point & designPoint)
    : EvaluationImplementation()
    , function_(function)
    , designPoint_(designPoint)
  {
    // Nothing to do
  }

  FrozenLimitStateEvaluation * clone() const override
  {
    return new FrozenLimitStateEvaluation(*this);
  }

  Point operator()(const Point & theta) const override
  {
    Function function(function_);
    function.setParameter(theta);
    return function(designPoint_);
  }

  UnsignedInteger getInputDimension() const override
  {
    return function_.getParameterDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return function_.getOutputDimension();
  }

  Description getInputDescription() const override
  {
    return function_.getParameterDescription();
  }

  Description getOutputDescription() const override
  {
    return function_.getOutputDescription();
  }

private:
  Function function_;
  Point designPoint_;
};

/* Gradient of the frozen limit-state function */
class FrozenLimitStateGradient
  : public GradientImplementation
{
public:
  FrozenLimitStateGradient(const Function & function,
                           const Point & designPoint)
    : GradientImplementation()
    , function_(function)
    , designPoint_(designPoint)
  {
    // Nothing to do
  }

  FrozenLimitStateGradient * clone() const override
  {
    return new FrozenLimitStateGradient(*this);
  }

  Matrix gradient(const Point & theta) const override
  {
    Function function(function_);
    function.setParameter(theta);
    // The parameter gradient already has the OpenTURNS orientation, that is
    // the input dimension (the parameter) by the output dimension.
    return function.parameterGradient(designPoint_);
  }

  UnsignedInteger getInputDimension() const override
  {
    return function_.getParameterDimension();
  }

  UnsignedInteger getOutputDimension() const override
  {
    return function_.getOutputDimension();
  }

private:
  Function function_;
  Point designPoint_;
};

}

namespace ProbabilityEstimatorTools
{

Point buildFullPoint(const Point & inputPoint,
                     const Point & theta,
                     const Indices & inputPositions,
                     const Indices & parametersPositions,
                     const UnsignedInteger fullDimension)
{
  Point fullPoint(fullDimension);
  for (UnsignedInteger i = 0; i < parametersPositions.getSize(); ++i)
    fullPoint[parametersPositions[i]] = theta[i];
  for (UnsignedInteger i = 0; i < inputPositions.getSize(); ++i)
    fullPoint[inputPositions[i]] = inputPoint[i];
  return fullPoint;
}

Point buildIndicators(const Point & limitStateValue)
{
  const UnsignedInteger dimension = limitStateValue.getSize();
  Point indicators(dimension);
  for (UnsignedInteger j = 0; j < dimension; ++j)
    indicators[j] = (limitStateValue[j] >= 0.0) ? 1.0 : 0.0;
  return indicators;
}

Point evaluateLimitState(const Function & limitState,
                         const Point & designPoint,
                         const Point & theta)
{
  const Evaluation evaluation(limitState.getEvaluation());
  const ParametricEvaluation * p_parametricEvaluation = dynamic_cast<const ParametricEvaluation *>(evaluation.getImplementation().get());
  if (p_parametricEvaluation)
  {
    const Function fullFunction(p_parametricEvaluation->getFunction());
    return fullFunction(buildFullPoint(designPoint, theta,
                                       p_parametricEvaluation->getInputPositions(),
                                       p_parametricEvaluation->getParametersPositions(),
                                       fullFunction.getInputDimension()));
  }
  Function function(limitState);
  function.setParameter(theta);
  return function(designPoint);
}

Scalar evaluatePredicate(const Function & predicate,
                         const Point & indicators)
{
  const Point value(predicate(indicators));
  if (value.getSize() != 1)
    throw InvalidDimensionException(HERE) << "The predicate must return one value, got " << value.getSize();
  const Scalar booleanValue = value[0];
  if ((booleanValue != 0.0) && (booleanValue != 1.0))
    throw InvalidArgumentException(HERE) << "The predicate must return 0 or 1, got " << booleanValue
                                         << " for the indicators " << indicators;
  return booleanValue;
}

void checkArguments(const Function & predicate,
                    const Function & limitState,
                    const Distribution & distribution,
                    const Point & designPoint)
{
  const UnsignedInteger limitStateOutputDimension = limitState.getOutputDimension();
  if (limitStateOutputDimension == 0)
    throw InvalidDimensionException(HERE) << "The limit-state function must have at least one output";
  if (predicate.getInputDimension() != limitStateOutputDimension)
    throw InvalidDimensionException(HERE) << "The predicate input dimension (" << predicate.getInputDimension()
                                          << ") must match the limit-state output dimension ("
                                          << limitStateOutputDimension << ")";
  if (predicate.getOutputDimension() != 1)
    throw InvalidDimensionException(HERE) << "The predicate output dimension (" << predicate.getOutputDimension()
                                          << ") must be 1";
  if (predicate.getParameterDimension() != 0)
    throw InvalidDimensionException(HERE) << "The predicate must not have a parameter, got a parameter of dimension "
                                          << predicate.getParameterDimension();
  if (limitState.getParameterDimension() != distribution.getDimension())
    throw InvalidDimensionException(HERE) << "The limit-state parameter dimension (" << limitState.getParameterDimension()
                                          << ") must match the distribution dimension ("
                                          << distribution.getDimension() << ")";
  if (designPoint.getDimension() != limitState.getInputDimension())
    throw InvalidDimensionException(HERE) << "The design point dimension (" << designPoint.getDimension()
                                          << ") must match the limit-state input dimension ("
                                          << limitState.getInputDimension() << ")";
  // Necessary condition only: a predicate that is Boolean at the probe can
  // still return another value elsewhere.  evaluatePredicate() checks every
  // value actually used by the estimator.
  (void) evaluatePredicate(predicate, Point(limitStateOutputDimension, 0.0));
}

Function buildFrozenLimitState(const Function & limitState,
                               const Point & designPoint)
{
  return Function(Evaluation(new FrozenLimitStateEvaluation(limitState, designPoint)),
                  Gradient(new FrozenLimitStateGradient(limitState, designPoint)),
                  Hessian(new NoHessian()));
}

} /* namespace ProbabilityEstimatorTools */
} /* namespace OTROBOPT */
