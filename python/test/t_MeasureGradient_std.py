#!/usr/bin/env python

import openturns as ot
import openturns.testing as ott
import otrobopt


def check_measure_gradient(measure, x, expected):
    """Check the measure gradient against its closed form and its finite
    difference."""
    function = otrobopt.MeasureFunction(measure)
    gradient = function.gradient(x)
    ott.assert_almost_equal(gradient, expected)
    # The gradient is the analytic measure gradient, not the finite difference
    # fallback installed by FunctionImplementation.
    gradientClass = function.getGradient().__repr__()
    assert "MeasureGradient" in gradientClass, gradientClass
    # Cross-check against a centered finite difference of the measure itself
    reference = ot.CenteredFiniteDifferenceGradient(1.0e-7, measure).gradient(x)
    ott.assert_almost_equal(gradient, reference, 1.0e-5, 1.0e-5)


# Mean measure: a normal parameter and a scalar function
thetaDistribution = ot.Normal(2.0, 1.0)
fullFunction = ot.SymbolicFunction(["x", "theta"], ["theta*x"])
function = ot.ParametricFunction(fullFunction, [1], [1.0])

mean = otrobopt.MeanMeasure(function, thetaDistribution)
check_measure_gradient(mean, [2.0], ot.Matrix([[2.0]]))
ott.assert_almost_equal(otrobopt.MeasureFunction(mean)([2.0]), [4.0])

# Variance measure: Var(theta*x) = x^2, derivative 2x
variance = otrobopt.VarianceMeasure(function, thetaDistribution)
check_measure_gradient(variance, [2.0], ot.Matrix([[4.0]]))
ott.assert_almost_equal(otrobopt.MeasureFunction(variance)([2.0]), [4.0])

# Mean-standard deviation tradeoff: 0.5*(2x) + 0.5*|x| = 1.5x at x=2
tradeoff = otrobopt.MeanStandardDeviationTradeoffMeasure(
    function, thetaDistribution, [0.5]
)
check_measure_gradient(tradeoff, [2.0], ot.Matrix([[1.5]]))
ott.assert_almost_equal(otrobopt.MeasureFunction(tradeoff)([2.0]), [3.0])

# Aggregated measure: the blocks are concatenated in the collection order
aggregated = otrobopt.AggregatedMeasure([mean, variance])
check_measure_gradient(aggregated, [2.0], ot.Matrix([[2.0, 4.0]]))
ott.assert_almost_equal(otrobopt.MeasureFunction(aggregated)([2.0]), [4.0, 4.0])

# Vector-valued function: one gradient column per output
vectorFunction = ot.SymbolicFunction(["x", "theta"], ["theta*x", "x^2"])
vectorParametric = ot.ParametricFunction(vectorFunction, [1], [1.0])
vectorMean = otrobopt.MeanMeasure(vectorParametric, thetaDistribution)
check_measure_gradient(vectorMean, [2.0], ot.Matrix([[2.0, 4.0]]))

# Discrete distribution: theta in {1, 3} with equal weights
discreteDistribution = ot.UserDefined([[1.0], [3.0]], [0.5, 0.5])
discreteMean = otrobopt.MeanMeasure(function, discreteDistribution)
check_measure_gradient(discreteMean, [2.0], ot.Matrix([[2.0]]))
discreteVariance = otrobopt.VarianceMeasure(function, discreteDistribution)
check_measure_gradient(discreteVariance, [2.0], ot.Matrix([[4.0]]))

# A measure without an analytic gradient keeps the finite difference fallback
quantile = otrobopt.QuantileMeasure(function, thetaDistribution, 0.9)
quantileFunction = otrobopt.MeasureFunction(quantile)
quantileGradient = quantileFunction.gradient([2.0])
assert "MeasureGradient" not in quantileFunction.getGradient().__repr__()
assert quantileGradient.getNbRows() == 1
assert quantileGradient.getNbColumns() == 1

# The robust problem objective now carries the analytic measure gradient
problem = otrobopt.RobustOptimizationProblem(mean, quantile)
ott.assert_almost_equal(problem.getObjective().gradient([2.0]), ot.Matrix([[2.0]]))

# A non-derivable aggregate falls back to finite differences as a whole
mixedAggregate = otrobopt.AggregatedMeasure([mean, quantile])
mixedFunction = otrobopt.MeasureFunction(mixedAggregate)
assert "MeasureGradient" not in mixedFunction.getGradient().__repr__()
assert mixedFunction.gradient([2.0]).getNbRows() == 1
