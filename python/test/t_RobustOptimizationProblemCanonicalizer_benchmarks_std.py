#!/usr/bin/env python

# Benchmarks for the canonical construction.
#
# Two closed-form problems are used:
#
# - a bivariate normal model whose marginal standard deviations and
#   correlation are optimized, with fixed means and a lower bound on the
#   standard deviations;
# - a uniform model whose upper support bound is optimized, which exercises
#   the non-regular case of a support depending on the parameter.

import openturns as ot
import openturns.testing as ott
import otrobopt


def cheap_quadrature(measure):
    """Keep the benchmark fast; the analytical point is used as reference."""
    measure.setIntegrationAlgorithm(
        ot.IteratedQuadrature(
            ot.GaussKronrod(
                3, 1.0e-2, ot.GaussKronrodRule(ot.GaussKronrodRule.G7K15)
            )
        )
    )
    return measure


# ------------------------------------------------------- correlated normal
def build_normal_problem():
    mean = [2.0, 2.0]
    correlation = ot.CorrelationMatrix(2)
    correlation[0, 1] = 0.0
    distribution = ot.Normal(mean, [1.0, 1.0], correlation)
    loss = ot.ParametricFunction(
        ot.SymbolicFunction(
            ["x1", "x2", "t1", "t2"],
            ["10 * (t1 - x1)^2 + (t2 - x2)^2"],
        ),
        [2, 3],
        [0.0, 0.0],
    )
    # The separation does not depend on the design variables (x1, x2), but
    # they must be declared so that the measure function input dimension
    # matches the design dimension expected by the canonicalizer.
    separation = ot.ParametricFunction(
        ot.SymbolicFunction(
            ["x1", "x2", "t1", "t2"],
            ["(t1 - t2)^2 - 0.6"],
        ),
        [2, 3],
        [0.0, 0.0],
    )
    objective = cheap_quadrature(otrobopt.MeanMeasure(loss, distribution))
    constraint = cheap_quadrature(otrobopt.MeanMeasure(separation, distribution))
    problem = otrobopt.RobustOptimizationProblem(objective, constraint)
    problem.setBounds(ot.Interval([0.0, 0.0], [4.0, 4.0]))

    options = otrobopt.CanonicalizationOptions()
    options.setActiveParameter([1, 3, 4])
    options.setParameterBounds(ot.Interval([0.2, 0.2, -0.8], [1.5, 1.5, 0.8]))
    options.setParameterStart([0.5, 0.8, -0.2])
    canonicalizer = otrobopt.RobustOptimizationProblemCanonicalizer(
        problem, options
    )
    return canonicalizer, loss, separation, mean


normalizer, normalLoss, normalSeparation, normalMean = build_normal_problem()
normalProblem = normalizer.getCanonicalProblem()
normalStart = normalizer.getCanonicalStartingPoint()
normalObjective = normalizer.getCanonicalObjectiveFunction()
normalConstraint = normalizer.getCanonicalConstraintFunction()

assert normalProblem.getDimension() == 5
assert normalizer.getActiveParameter() == [1, 3, 4]
assert normalizer.usesStandardSpace()
assert normalizer.getReferenceDistribution() == ot.Normal(2)
ott.assert_almost_equal(normalStart, [2.0, 2.0, 0.5, 0.8, -0.2])

# Analytical solution: negative correlation at its lower bound, expensive
# component at its lower standard-deviation bound.
analyticSigma1 = 0.2
analyticRho = -0.8
analyticSigma2 = analyticRho * analyticSigma1 + (
    0.6 - (1.0 - analyticRho**2) * analyticSigma1**2
) ** 0.5
analyticNormalPoint = [2.0, 2.0, analyticSigma1, analyticSigma2, analyticRho]
analyticNormalObjective = 10.0 * analyticSigma1**2 + analyticSigma2**2

ott.assert_almost_equal(
    normalObjective(analyticNormalPoint),
    [analyticNormalObjective],
    1.0e-6,
    1.0e-8,
)
ott.assert_almost_equal(
    normalConstraint(analyticNormalPoint), [0.0], 1.0e-4, 1.0e-4
)

# The canonical measures must agree with a freshly parameterized distribution.
freshCorrelation = ot.CorrelationMatrix(2)
freshCorrelation[0, 1] = analyticRho
freshNormal = ot.Normal(
    normalMean, [analyticSigma1, analyticSigma2], freshCorrelation
)
freshNormalObjective = otrobopt.MeanMeasure(normalLoss, freshNormal)
freshNormalConstraint = otrobopt.MeanMeasure(normalSeparation, freshNormal)
ott.assert_almost_equal(
    normalObjective(analyticNormalPoint),
    freshNormalObjective([2.0, 2.0]),
    1.0e-4,
    1.0e-6,
)
ott.assert_almost_equal(
    normalConstraint(analyticNormalPoint),
    freshNormalConstraint([2.0, 2.0]),
    1.0e-4,
    1.0e-4,
)

normalSolver = ot.Cobyla()
normalSolver.setProblem(normalProblem)
normalSolver.setStartingPoint(normalStart)
normalSolver.setMaximumIterationNumber(20)
normalSolver.setCheckStatus(False)
normalSolver.run()
normalOptimum = normalSolver.getResult().getOptimalPoint()
ott.assert_almost_equal(normalOptimum, analyticNormalPoint, 1.0e-3, 1.0e-6)
ott.assert_almost_equal(
    normalSolver.getResult().getOptimalValue(),
    [analyticNormalObjective],
    1.0e-4,
    1.0e-6,
)
assert abs(normalOptimum[2] - 0.2) < 1.0e-6, normalOptimum
assert abs(normalOptimum[4] + 0.8) < 1.0e-6, normalOptimum
assert normalConstraint(normalOptimum)[0] >= -1.0e-8, normalOptimum


# ----------------------------------------------------------- uniform bound
def build_uniform_problem():
    distribution = ot.Uniform(0.0, 1.0)
    loss = ot.ParametricFunction(
        ot.SymbolicFunction(["x", "theta"], ["(theta - x)^2"]), [1], [0.0]
    )
    # The threshold does not depend on the design variable x, but x must be
    # declared so that the measure function input dimension matches the
    # design dimension expected by the canonicalizer.
    threshold = ot.ParametricFunction(
        ot.SymbolicFunction(["x", "theta"], ["theta - 0.3"]),
        [1],
        [0.0],
    )
    objective = otrobopt.MeanMeasure(loss, distribution)
    constraint = otrobopt.JointChanceMeasure(
        threshold, distribution, ot.GreaterOrEqual(), 0.9
    )
    problem = otrobopt.RobustOptimizationProblem(objective, constraint)
    problem.setBounds(ot.Interval([0.0], [5.0]))

    options = otrobopt.CanonicalizationOptions()
    options.setActiveParameter([1])
    options.setParameterBounds(ot.Interval([0.1], [5.0]))
    options.setParameterStart([1.0])
    canonicalizer = otrobopt.RobustOptimizationProblemCanonicalizer(
        problem, options
    )
    return canonicalizer, threshold


uniformizer, uniformThreshold = build_uniform_problem()
uniformProblem = uniformizer.getCanonicalProblem()
uniformStart = uniformizer.getCanonicalStartingPoint()
uniformObjective = uniformizer.getCanonicalObjectiveFunction()
uniformConstraint = uniformizer.getCanonicalConstraintFunction()

assert uniformProblem.getDimension() == 2
assert uniformizer.getActiveParameter() == [1]
assert uniformizer.usesStandardSpace()
assert uniformizer.getReferenceDistribution() == ot.Normal()
ott.assert_almost_equal(uniformStart, [2.5, 1.0])

# Analytical solution: b = 0.3 / (1 - 0.9) = 3, x = b / 2 = 1.5.
analyticUniformPoint = [1.5, 3.0]
ott.assert_almost_equal(uniformObjective(analyticUniformPoint), [0.75], 1.0e-10, 1.0e-12)
ott.assert_almost_equal(uniformConstraint(analyticUniformPoint), [0.0], 1.0e-10, 1.0e-10)

# The canonical objective is b^2/12 + (b/2 - x)^2 away from the optimum too.
comparisonPoint = [1.5, 2.9]
ott.assert_almost_equal(
    uniformObjective(comparisonPoint),
    [2.9**2 / 12.0 + (2.9 / 2.0 - 1.5) ** 2],
    1.0e-10,
    1.0e-12,
)

# The canonical chance constraint is Phi^-1(P) - Phi^-1(0.9) and must agree
# with a freshly parameterized distribution.  It is negative below b = 3.
freshUniform = ot.Uniform(0.0, 2.9)
freshChance = otrobopt.JointChanceMeasure(
    uniformThreshold, freshUniform, ot.GreaterOrEqual(), 0.9
)
ott.assert_almost_equal(
    uniformConstraint(comparisonPoint), freshChance([1.5]), 1.0e-8, 1.0e-8
)
assert uniformConstraint(comparisonPoint)[0] < 0.0
assert uniformConstraint([1.5, 3.1])[0] > 0.0

# Probability at the analytical optimum.
assert abs((1.0 - 0.3 / 3.0) - 0.9) < 1.0e-15

# The support moves with b: the naive fixed-support density term misses the
# boundary flux, and the two terms recover the finite difference.
step = 1.0e-6
finiteDifference = (
    (1.0 - 0.3 / (3.0 + step)) - (1.0 - 0.3 / (3.0 - step))
) / (2.0 * step)
densityTerm = -(3.0 - 0.3) / 3.0**2
boundaryTerm = 1.0 / 3.0
assert abs(finiteDifference - 0.3 / 3.0**2) < 1.0e-9
assert abs(densityTerm + boundaryTerm - finiteDifference) < 1.0e-9

uniformSolver = ot.Cobyla()
uniformSolver.setProblem(uniformProblem)
uniformSolver.setStartingPoint(uniformStart)
uniformSolver.setMaximumIterationNumber(20)
uniformSolver.setCheckStatus(False)
uniformSolver.run()
uniformOptimum = uniformSolver.getResult().getOptimalPoint()
ott.assert_almost_equal(uniformOptimum, analyticUniformPoint, 1.0e-3, 1.0e-6)
ott.assert_almost_equal(
    uniformSolver.getResult().getOptimalValue(), [0.75], 1.0e-3, 1.0e-6
)
assert uniformConstraint(uniformOptimum)[0] >= -1.0e-5, uniformOptimum
assert abs(uniformOptimum[1] - 3.0) < 1.0e-3, uniformOptimum
