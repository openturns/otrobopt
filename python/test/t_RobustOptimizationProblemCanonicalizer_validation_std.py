#!/usr/bin/env python

# End-to-end validation of the canonical transformation.
#
# Fast-path families are checked against a freshly parameterized distribution.
# The generic Student path is expected to fail until the OpenTURNS defects
# tracked by issues #3418-#3424 (PR #3422) are fixed: those checks are written
# as explicit xfails that fail the test if they start passing.

import time

import openturns as ot
import openturns.testing as ott
import otrobopt

OPENTURNS_ISSUES = "#3418-#3424 / PR #3422"


def canonicalize(measure, options=None):
    problem = otrobopt.RobustOptimizationProblem(measure, ot.Function())
    if options is None:
        return otrobopt.RobustOptimizationProblemCanonicalizer(problem)
    return otrobopt.RobustOptimizationProblemCanonicalizer(problem, options)


def with_parameter(distribution, parameter):
    result = type(distribution)(distribution)
    result.setParameter(parameter)
    return result


def assert_measure_close(actual, expected, tolerance):
    """Assert closeness using AssertionError so that xfail can intercept it."""
    actual = list(actual)
    expected = list(expected)
    assert len(actual) == len(expected), (actual, expected)
    for i, (a, e) in enumerate(zip(actual, expected)):
        assert abs(a - e) <= tolerance, (
            "component {}: canonical {!r} != fresh {!r} "
            "(difference {:.3g} > {:.3g})".format(i, a, e, abs(a - e), tolerance)
        )


def check_measure(factory, distribution, parameter, xValues, tolerance):
    measure = factory(distribution)
    canonicalizer = canonicalize(measure)
    canonicalMeasure = canonicalizer.getCanonicalProblem().getRobustnessMeasure()
    freshMeasure = factory(with_parameter(distribution, parameter))
    for x in xValues:
        assert_measure_close(
            canonicalMeasure([x] + list(parameter)),
            freshMeasure([x]),
            tolerance,
        )
    return canonicalizer


def xfail(description, check):
    """Expect check() to fail because of the known OpenTURNS defects."""
    try:
        check()
    except AssertionError as exc:
        message = str(exc).replace("\n", " ")
        print("xfail: {} ({}): {}".format(description, OPENTURNS_ISSUES, message[:200]))
    else:
        raise AssertionError(
            "unexpected pass: {} now satisfies its check. The OpenTURNS defect "
            "tracked by {} may be fixed: remove this xfail and update the "
            "report.".format(description, OPENTURNS_ISSUES)
        )


# ------------------------------------------------------- fast-path families
# A scalar parametric function whose nonlinear dependence on the realization
# exercises the mean, the variance and the quantile.
fScalar = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta"], ["x*theta + theta^2"]), [1], [1.0]
)

fastPathFamilies = [
    ("Normal", ot.Normal(2.0, 0.3), [4.0, 0.5]),
    ("Uniform", ot.Uniform(0.0, 2.0), [-1.0, 3.0]),
    ("Gamma", ot.Gamma(2.0, 1.0, 0.0), [3.0, 1.5, 0.5]),
    ("Beta", ot.Beta(2.0, 3.0, 0.0, 1.0), [3.0, 2.0, -1.0, 2.0]),
    ("LogNormal", ot.LogNormal(0.0, 1.0, 0.0), [1.0, 0.5, 0.2]),
    ("Exponential", ot.Exponential(1.0, 0.0), [2.0, 0.5]),
    ("Gumbel", ot.Gumbel(1.0, 0.0), [2.0, 0.5]),
    ("Logistic", ot.Logistic(0.0, 1.0), [1.0, 2.0]),
    ("WeibullMin", ot.WeibullMin(1.0, 1.0, 0.0), [2.0, 1.5, 0.2]),
    ("Triangular", ot.Triangular(0.0, 0.5, 1.0), [0.2, 0.6, 1.5]),
]

for name, distribution, parameter in fastPathFamilies:
    for label, factory, tolerance in [
        ("mean", lambda d: otrobopt.MeanMeasure(fScalar, d), 1.0e-6),
        ("variance", lambda d: otrobopt.VarianceMeasure(fScalar, d), 1.0e-6),
        ("quantile", lambda d: otrobopt.QuantileMeasure(fScalar, d, 0.9), 2.0e-2),
    ]:
        canonicalizer = check_measure(
            factory, distribution, parameter, [1.5], tolerance
        )
        assert canonicalizer.usesStandardSpace(), (name, label)
        assert canonicalizer.getReferenceDistribution() == ot.Normal(1), (name, label)

# ------------------------------------------------ correlated normal (copula)
correlation = ot.CorrelationMatrix(2)
correlation[0, 1] = 0.4
normal2 = ot.Normal([1.0, -1.0], [0.5, 2.0], correlation)
fNormal2 = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta1", "theta2"], ["x*theta1 + theta2^2"]),
    [1, 2],
    normal2.getMean(),
)
normal2Parameter = [2.0, 1.5, 3.0, 0.7, -0.3]
for label, factory in [
    ("mean", lambda d: otrobopt.MeanMeasure(fNormal2, d)),
    ("variance", lambda d: otrobopt.VarianceMeasure(fNormal2, d)),
]:
    canonicalizer = check_measure(
        factory, normal2, normal2Parameter, [1.0], 1.0e-6
    )
    assert canonicalizer.usesStandardSpace(), label
    assert canonicalizer.getReferenceDistribution() == ot.Normal(2), label

# ------------------------------------------------- uniform worst-case values
# The standard fast path maps the standard normal reference to the uniform
# distribution.  The worst case of f(x; theta) = x * theta over theta in
# [-1, 2] is -1 at x = 1 and -2 at x = -1.
fWorst = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta"], ["x*theta"]), [1], [1.0]
)
uniform = ot.Uniform(-1.0, 2.0)
canonicalizer = canonicalize(otrobopt.WorstCaseMeasure(fWorst, uniform, True))
assert canonicalizer.usesStandardSpace()
canonicalWorst = canonicalizer.getCanonicalProblem().getRobustnessMeasure()
ott.assert_almost_equal(canonicalWorst([1.0, -1.0, 2.0]), [-1.0], 1.0e-4, 1.0e-4)
ott.assert_almost_equal(canonicalWorst([-1.0, -1.0, 2.0]), [-2.0], 1.0e-4, 1.0e-4)

# ------------------------------------------------------- Student scale xfails
# The mean is linear in the realization and remains correct even with the
# defective generic transformation.  The variance, the quantile and the
# chance measures depend on the pushforward law and are wrong until Stage 3.
student = ot.Student(5.0, 0.0, 1.0)
studentParameter = [8.0, 1.0, 2.0]
fStudent = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta"], ["x*theta"]), [1], [1.0]
)
studentMean = otrobopt.MeanMeasure(fStudent, student)
canonicalizer = canonicalize(studentMean)
assert not canonicalizer.usesStandardSpace()
assert canonicalizer.getReferenceDistribution() == ot.Normal(1)
canonicalStudentMean = canonicalizer.getCanonicalProblem().getRobustnessMeasure()
freshStudentMean = otrobopt.MeanMeasure(
    fStudent, with_parameter(student, studentParameter)
)
ott.assert_almost_equal(
    canonicalStudentMean([1.5] + studentParameter),
    freshStudentMean([1.5]),
    1.0e-6,
    1.0e-6,
)

xfail(
    "Student 1D scale variance",
    lambda: check_measure(
        lambda d: otrobopt.VarianceMeasure(fStudent, d),
        student,
        studentParameter,
        [1.5],
        1.0e-6,
    ),
)
xfail(
    "Student 1D scale 90% quantile",
    lambda: check_measure(
        lambda d: otrobopt.QuantileMeasure(fStudent, d, 0.9),
        student,
        studentParameter,
        [1.5],
        1.0e-4,
    ),
)
xfail(
    "Student 1D scale joint chance",
    lambda: check_measure(
        lambda d: otrobopt.JointChanceMeasure(fStudent, d, ot.GreaterOrEqual(), 0.9),
        student,
        studentParameter,
        [1.5],
        1.0e-4,
    ),
)

# Elliptical shape: a multivariate Student with a non-diagonal correlation.
student2 = ot.Student(5.0, [0.0, 1.0], [1.0, 2.0], correlation)
fStudent2 = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta1", "theta2"], ["x*theta1 + theta2"]),
    [1, 2],
    [0.0, 1.0],
)
student2Parameter = [8.0, 1.0, 1.0, 1.5, 2.0, 0.2]


def student2Variance(distribution):
    measure = otrobopt.VarianceMeasure(fStudent2, distribution)
    measure.setIntegrationAlgorithm(
        ot.IteratedQuadrature(
            ot.GaussKronrod(
                20, 1.0e-4, ot.GaussKronrodRule(ot.GaussKronrodRule.G7K15)
            )
        )
    )
    return measure


xfail(
    "Student 2D elliptical variance",
    lambda: check_measure(
        student2Variance,
        student2,
        student2Parameter,
        [1.0],
        1.0e-2,
    ),
)

# ------------------------------------- chance constraint with linear limit state
# Minimize E[x - theta] = x - mu subject to P(x - theta >= 0) >= 0.9 with
# theta ~ Normal(mu, sigma).  The constraint is (x - mu) / sigma >= z_0.9,
# so the optimum satisfies x = mu + sigma z_0.9 and the optimal value is
# sigma z_0.9.  With mu in [1, 2] and sigma in [0.1, 0.5], the optimal scale
# is sigma = 0.1; mu is not unique because the reduced objective depends only
# on sigma.  The test checks the invariant x = mu + sigma z_0.9.
z90 = ot.Normal().computeQuantile(0.9)[0]
theta0 = ot.Normal(2.0, 0.1)
linearLimitState = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta"], ["x - theta"]), [1], [2.0]
)
meanMeasure = otrobopt.MeanMeasure(linearLimitState, theta0)
chanceMeasure = otrobopt.JointChanceMeasure(
    linearLimitState, theta0, ot.GreaterOrEqual(), 0.9
)
problem = otrobopt.RobustOptimizationProblem(meanMeasure, chanceMeasure)
problem.setBounds(ot.Interval([0.0], [5.0]))
options = otrobopt.CanonicalizationOptions()
options.setParameterBounds(ot.Interval([1.0, 0.1], [2.0, 0.5]))
options.setParameterStart([2.0, 0.1])
canonicalizer = otrobopt.RobustOptimizationProblemCanonicalizer(problem, options)
canonicalProblem = canonicalizer.getCanonicalProblem()
assert canonicalizer.getOptimizeParameter()
assert canonicalizer.usesStandardSpace()
ott.assert_almost_equal(canonicalProblem.getDimension(), 3)
ott.assert_almost_equal(
    canonicalizer.getCanonicalStartingPoint(), [2.5, 2.0, 0.1]
)
ott.assert_almost_equal(
    canonicalProblem.getBounds().getLowerBound(), [0.0, 1.0, 0.1]
)
ott.assert_almost_equal(
    canonicalProblem.getBounds().getUpperBound(), [5.0, 2.0, 0.5]
)

canonicalMean = canonicalProblem.getRobustnessMeasure()
canonicalChance = canonicalizer.getCanonicalConstraintFunction()
for x, mu, sigma in [(2.5, 2.0, 0.1), (3.0, 1.5, 0.5), (2.0, 1.0, 0.5)]:
    ott.assert_almost_equal(
        canonicalMean([x, mu, sigma]), [x - mu], 1.0e-10, 1.0e-10
    )
    ott.assert_almost_equal(
        canonicalChance([x, mu, sigma]),
        [(x - mu) / sigma - z90],
        1.0e-4,
        1.0e-4,
    )

solver = ot.Cobyla()
solver.setProblem(canonicalProblem)
solver.setStartingPoint(canonicalizer.getCanonicalStartingPoint())
solver.run()
result = solver.getResult()
optimum = result.getOptimalPoint()
ott.assert_almost_equal(result.getOptimalValue(), [z90 * 0.1], 1.0e-4, 1.0e-4)
assert 1.0 <= optimum[1] <= 2.0
ott.assert_almost_equal(optimum[2], 0.1, 1.0e-6, 1.0e-6)
ott.assert_almost_equal(optimum[0], optimum[1] + z90 * optimum[2], 1.0e-5, 1.0e-5)
assert canonicalChance(optimum)[0] >= -1.0e-6
assert optimum[2] >= 0.1
assert optimum[2] <= 0.5

# ------------------------------------------- frozen parameter and direct solve
# With the parameter frozen at the original value, the canonical problem has
# dimension one and must reproduce the direct solution of the original
# robust problem.
frozenOptions = otrobopt.CanonicalizationOptions()
frozenOptions.setOptimizeParameter(False)
frozenOptions.setParameterStart([2.0, 0.1])
frozenCanonicalizer = otrobopt.RobustOptimizationProblemCanonicalizer(
    problem, frozenOptions
)
frozenProblem = frozenCanonicalizer.getCanonicalProblem()
assert not frozenCanonicalizer.getOptimizeParameter()
ott.assert_almost_equal(frozenProblem.getDimension(), 1)
ott.assert_almost_equal(
    frozenCanonicalizer.getCanonicalStartingPoint(), [2.5]
)

directSolver = ot.Cobyla()
directSolver.setProblem(problem)
directSolver.setStartingPoint([2.5])
directSolver.run()
directResult = directSolver.getResult()

frozenSolver = ot.Cobyla()
frozenSolver.setProblem(frozenProblem)
frozenSolver.setStartingPoint(frozenCanonicalizer.getCanonicalStartingPoint())
frozenSolver.run()
frozenResult = frozenSolver.getResult()
ott.assert_almost_equal(
    frozenResult.getOptimalPoint(),
    directResult.getOptimalPoint(),
    1.0e-5,
    1.0e-5,
)
ott.assert_almost_equal(
    frozenResult.getOptimalValue(),
    directResult.getOptimalValue(),
    1.0e-5,
    1.0e-5,
)

# --------------------------------------------- parameter bounds are enforced
# A positive lower bound on the scale moves the optimum to sigma = 0.2 and
# x = mu + 0.2 z_0.9, while keeping the parameter inside the user bounds.
boundedOptions = otrobopt.CanonicalizationOptions()
boundedOptions.setParameterBounds(ot.Interval([1.0, 0.2], [2.0, 0.5]))
boundedOptions.setParameterStart([2.0, 0.2])
boundedCanonicalizer = otrobopt.RobustOptimizationProblemCanonicalizer(
    problem, boundedOptions
)
boundedProblem = boundedCanonicalizer.getCanonicalProblem()
boundedSolver = ot.Cobyla()
boundedSolver.setProblem(boundedProblem)
boundedSolver.setStartingPoint(boundedCanonicalizer.getCanonicalStartingPoint())
boundedSolver.run()
boundedOptimum = boundedSolver.getResult().getOptimalPoint()
assert 1.0 <= boundedOptimum[1] <= 2.0
ott.assert_almost_equal(boundedOptimum[2], 0.2, 1.0e-6, 1.0e-6)
ott.assert_almost_equal(
    boundedOptimum[0],
    boundedOptimum[1] + z90 * boundedOptimum[2],
    1.0e-5,
    1.0e-5,
)
ott.assert_almost_equal(
    boundedSolver.getResult().getOptimalValue(), [z90 * 0.2], 1.0e-4, 1.0e-4
)
assert boundedOptimum[2] >= 0.2
assert boundedOptimum[2] <= 0.5

# ------------------------------------------------------- transformation cache
# Rebuilding T_s for every new parameter is more expensive than reusing the
# cached transformation for a fixed parameter.
transformation = otrobopt.ParametricDistributionTransformation(
    ot.Normal(2.0, 0.1), ot.Normal(1)
)
transformation.setParameter([2.0, 0.1])
sampleCount = 20000
start = time.perf_counter()
for _ in range(sampleCount):
    transformation([0.5])
fixedTime = time.perf_counter() - start
start = time.perf_counter()
for i in range(sampleCount):
    transformation.setParameter([2.0 + 1.0e-8 * i, 0.1 + 1.0e-9 * i])
    transformation([0.5])
changingTime = time.perf_counter() - start
print(
    "transformation timing: fixed={:.3f}s changing={:.3f}s ratio={:.2f}".format(
        fixedTime, changingTime, changingTime / fixedTime
    )
)
assert fixedTime < changingTime

print("Stage 5 validation passed")
