#!/usr/bin/env python

import openturns as ot
import openturns.testing as ott
import otrobopt


def make_function(distribution):
    # f(x; theta) = x * theta with a scalar parameter
    return ot.ParametricFunction(
        ot.SymbolicFunction(["x", "theta"], ["x*theta"]),
        [1],
        [distribution.getParameter()[0]],
    )


def canonicalize(measure, options=None):
    problem = otrobopt.RobustOptimizationProblem(measure, ot.Function())
    if options is None:
        return otrobopt.RobustOptimizationProblemCanonicalizer(problem)
    return otrobopt.RobustOptimizationProblemCanonicalizer(problem, options)


# ---------------------------------------------------------------- options
options = otrobopt.CanonicalizationOptions()
assert options.getOptimizeParameter()
assert options.getWorstCasePolicy() == otrobopt.CanonicalizationOptions.AUTO
assert not options.hasReference()
assert options.getReference() == ot.Uniform(-1.0, 1.0)
assert options.getParameterBounds().getDimension() == 0
assert options.getParameterStart().getDimension() == 0
assert options.getWorstCaseDomain().getDimension() == 0

# Python properties call the accessors and accept plain sequences
options.reference = ot.Normal(3.0, 2.0)
options.parameterStart = [1.0, 2.0]
options.parameterBounds = ot.Interval([0.0, 0.0], [5.0, 5.0])
options.worstCaseDomain = ot.Interval([-1.0], [2.0])
options.optimizeParameter = False
options.worstCasePolicy = otrobopt.CanonicalizationOptions.USER_DOMAIN
assert options.hasReference()
assert options.getReference() == ot.Normal(3.0, 2.0)
ott.assert_almost_equal(options.parameterStart, [1.0, 2.0])
ott.assert_almost_equal(options.parameterBounds.getLowerBound(), [0.0, 0.0])
ott.assert_almost_equal(options.parameterBounds.getUpperBound(), [5.0, 5.0])
ott.assert_almost_equal(options.worstCaseDomain.getLowerBound(), [-1.0])
ott.assert_almost_equal(options.worstCaseDomain.getUpperBound(), [2.0])
assert not options.getOptimizeParameter()
assert options.getWorstCasePolicy() == otrobopt.CanonicalizationOptions.USER_DOMAIN

# The accessors return copies
options.reference.setParameter([0.0, 1.0])
assert options.getReference() == ot.Normal(3.0, 2.0)

# The default Uniform(-1, 1) can be selected explicitly as reference
explicitDefault = otrobopt.CanonicalizationOptions()
explicitDefault.setReference(ot.Uniform(-1.0, 1.0))
assert explicitDefault.hasReference()
explicitDefault.clearReference()
assert not explicitDefault.hasReference()
with ott.assert_raises(TypeError):
    explicitDefault.setWorstCasePolicy(99)
assert "CanonicalizationOptions" in repr(explicitDefault)

# --------------------------------------------------- reference selection
normal = ot.Normal(2.0, 0.1)
f = make_function(normal)

# AUTO uses the parameter-free standard distribution when it exists
canonicalizer = canonicalize(otrobopt.MeanMeasure(f, normal))
assert canonicalizer.usesStandardSpace()
assert canonicalizer.getReferenceDistribution() == ot.Normal(1)
assert canonicalizer.getOptimizeParameter()
problem = canonicalizer.getCanonicalProblem()
ott.assert_almost_equal(problem.getDimension(), 3)
ott.assert_almost_equal(canonicalizer.getCanonicalStartingPoint(), [0.0, 2.0, 0.1])
assert "RobustOptimizationProblemCanonicalizer" in repr(canonicalizer)

# A uniform also has a parameter-free standard distribution
uniform = ot.Uniform(-1.0, 2.0)
canonicalizer = canonicalize(otrobopt.MeanMeasure(make_function(uniform), uniform))
assert canonicalizer.usesStandardSpace()
assert canonicalizer.getReferenceDistribution() == ot.Normal(1)

# A Student distribution uses the generic Normal(dim) reference
student = ot.Student(5.0, 2.0, 1.0)
studentMeasure = otrobopt.MeanMeasure(make_function(student), student)
canonicalizer = canonicalize(studentMeasure)
assert not canonicalizer.usesStandardSpace()
assert canonicalizer.getReferenceDistribution() == ot.Normal(1)

# AUTO honours an explicit reference
options = otrobopt.CanonicalizationOptions()
options.setReference(ot.Normal(0.0, 1.0))
canonicalizer = canonicalize(studentMeasure, options)
assert not canonicalizer.usesStandardSpace()
assert canonicalizer.getReferenceDistribution() == ot.Normal(0.0, 1.0)
options.reference = ot.Normal(1)
canonicalizer = canonicalize(studentMeasure, options)
assert not canonicalizer.usesStandardSpace()
assert canonicalizer.getReferenceDistribution() == ot.Normal(1)

# STANDARD requires the fast path and no explicit reference
options = otrobopt.CanonicalizationOptions()
options.setWorstCasePolicy(otrobopt.CanonicalizationOptions.STANDARD)
canonicalizer = canonicalize(otrobopt.MeanMeasure(f, normal), options)
assert canonicalizer.usesStandardSpace()
with ott.assert_raises(TypeError):
    canonicalize(studentMeasure, options)
options.setReference(ot.Normal(1))
with ott.assert_raises(TypeError):
    canonicalize(otrobopt.MeanMeasure(f, normal), options)

# NORMAL forces the standard normal reference
options = otrobopt.CanonicalizationOptions()
options.setWorstCasePolicy(otrobopt.CanonicalizationOptions.NORMAL)
canonicalizer = canonicalize(otrobopt.MeanMeasure(f, normal), options)
assert not canonicalizer.usesStandardSpace()
assert canonicalizer.getReferenceDistribution() == ot.Normal(1)
options.setReference(ot.Normal(1))
with ott.assert_raises(TypeError):
    canonicalize(otrobopt.MeanMeasure(f, normal), options)

# USER_REFERENCE requires a reference
options = otrobopt.CanonicalizationOptions()
options.setWorstCasePolicy(otrobopt.CanonicalizationOptions.USER_REFERENCE)
with ott.assert_raises(TypeError):
    canonicalize(studentMeasure, options)
options.setReference(ot.Uniform(-1.0, 1.0))
canonicalizer = canonicalize(studentMeasure, options)
assert canonicalizer.getReferenceDistribution() == ot.Uniform(-1.0, 1.0)

# USER_DOMAIN requires a domain
options = otrobopt.CanonicalizationOptions()
options.setWorstCasePolicy(otrobopt.CanonicalizationOptions.USER_DOMAIN)
with ott.assert_raises(TypeError):
    canonicalize(studentMeasure, options)

# Invalid references
options = otrobopt.CanonicalizationOptions()
options.setReference(ot.Normal(2))
with ott.assert_raises(RuntimeError):
    canonicalize(studentMeasure, options)
options.setReference(ot.Poisson(2.0))
with ott.assert_raises(TypeError):
    canonicalize(studentMeasure, options)

# ------------------------------------------- measure canonicalization
# The canonical measure at (x, s) must match a fresh measure with D_s
for measure in [
    otrobopt.MeanMeasure(f, normal),
    otrobopt.VarianceMeasure(f, normal),
    otrobopt.QuantileMeasure(f, normal, 0.9),
    otrobopt.JointChanceMeasure(f, normal, ot.GreaterOrEqual(), 0.9),
    otrobopt.IndividualChanceMeasure(f, normal, ot.GreaterOrEqual(), [0.9]),
    otrobopt.MeanStandardDeviationTradeoffMeasure(f, normal, [0.8]),
]:
    canonicalizer = canonicalize(measure)
    canonicalMeasure = canonicalizer.getCanonicalProblem().getRobustnessMeasure()
    isChance = isinstance(
        measure, (otrobopt.JointChanceMeasure, otrobopt.IndividualChanceMeasure)
    )
    tolerance = 0.02 if isChance else 1.0e-6
    for x in [1.0, 3.0]:
        for mu, sigma in [(2.0, 0.1), (5.0, 0.2)]:
            freshDistribution = ot.Normal(mu, sigma)
            if isinstance(measure, otrobopt.MeanMeasure):
                freshMeasure = otrobopt.MeanMeasure(f, freshDistribution)
            elif isinstance(measure, otrobopt.VarianceMeasure):
                freshMeasure = otrobopt.VarianceMeasure(f, freshDistribution)
            elif isinstance(measure, otrobopt.QuantileMeasure):
                freshMeasure = otrobopt.QuantileMeasure(f, freshDistribution, 0.9)
            elif isinstance(measure, otrobopt.JointChanceMeasure):
                freshMeasure = otrobopt.JointChanceMeasure(
                    f, freshDistribution, ot.GreaterOrEqual(), 0.9
                )
            elif isinstance(measure, otrobopt.IndividualChanceMeasure):
                freshMeasure = otrobopt.IndividualChanceMeasure(
                    f, freshDistribution, ot.GreaterOrEqual(), [0.9]
                )
            else:
                freshMeasure = otrobopt.MeanStandardDeviationTradeoffMeasure(
                    f, freshDistribution, [0.8]
                )
            ott.assert_almost_equal(
                canonicalMeasure([x, mu, sigma]),
                freshMeasure([x]),
                tolerance,
                tolerance,
            )

# A vector-valued limit state is a valid joint chance measure: the
# canonicalizer checks the measure output dimension, not the function one.
vectorFunction = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "t1", "t2"], ["t1 - x", "t2 - x"]),
    [1, 2],
    [2.0, 3.0],
)
vectorDistribution = ot.Normal([2.0, 3.0], [0.1, 0.2], ot.IdentityMatrix(2))
vectorChance = otrobopt.JointChanceMeasure(
    vectorFunction, vectorDistribution, ot.GreaterOrEqual(), 0.9
)
assert vectorChance.getFunction().getOutputDimension() == 2
assert vectorChance.getOutputDimension() == 1
canonicalizer = canonicalize(vectorChance)
canonicalChance = canonicalizer.getCanonicalProblem().getRobustnessMeasure()
assert canonicalChance.getOutputDimension() == 1
for a in [1.0, 2.5]:
    freshChance = otrobopt.JointChanceMeasure(
        vectorFunction,
        ot.Normal([2.0, 3.0], [0.1, 0.2], ot.IdentityMatrix(2)),
        ot.GreaterOrEqual(),
        0.9,
    )
    ott.assert_almost_equal(
        canonicalChance([a, 2.0, 0.1, 3.0, 0.2]),
        freshChance([a]),
        0.02,
        0.02,
    )

# The canonicalizer keeps a private copy of the reference
canonicalizer = canonicalize(otrobopt.MeanMeasure(f, normal))
canonicalMeasure = canonicalizer.getCanonicalProblem().getRobustnessMeasure()
returnedReference = canonicalizer.getReferenceDistribution()
returnedReference.setParameter([10.0, 2.0])
ott.assert_almost_equal(canonicalMeasure([1.0, 5.0, 0.2]), [5.0])
ott.assert_almost_equal(canonicalizer.getReferenceDistribution(), ot.Normal(1))

# Aggregated measures are canonicalized child by child
normal2 = ot.Normal([2.0, 3.0], [0.1, 0.2], ot.IdentityMatrix(2))
f2 = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "t1", "t2"], ["x*t1", "x*t2"]),
    [1, 2],
    [2.0, 3.0],
)
mean2 = otrobopt.MeanMeasure(f2, normal2)
variance2 = otrobopt.VarianceMeasure(f2, normal2)
aggregate = otrobopt.AggregatedMeasure([mean2, variance2])
objective = ot.SymbolicFunction(["x"], ["x^2"])
problem = otrobopt.RobustOptimizationProblem(objective, aggregate)
canonicalizer = otrobopt.RobustOptimizationProblemCanonicalizer(problem)
canonicalAggregate = canonicalizer.getCanonicalProblem().getReliabilityMeasure()
# The correlation block is inactive by default, so the canonical vector is
# x plus the two means and the two standard deviations.
assert canonicalizer.getActiveParameter() == [0, 1, 2, 3]
ott.assert_almost_equal(canonicalizer.getCanonicalProblem().getDimension(), 5)
ott.assert_almost_equal(canonicalizer.getCanonicalConstraintFunction().getInputDimension(), 5)
ott.assert_almost_equal(canonicalizer.getCanonicalConstraintFunction().getOutputDimension(), 4)
parameter2 = [2.0, 0.1, 3.0, 0.2]
ott.assert_almost_equal(
    canonicalAggregate([1.0] + parameter2), aggregate([1.0]), 1.0e-6, 1.0e-6
)
ott.assert_almost_equal(
    canonicalAggregate([2.0] + parameter2), aggregate([2.0]), 1.0e-6, 1.0e-6
)

# Activating the full parameter adds the correlation coordinate.
fullOptions = otrobopt.CanonicalizationOptions()
fullOptions.setActiveParameter([0, 1, 2, 3, 4])
fullCanonicalizer = otrobopt.RobustOptimizationProblemCanonicalizer(
    problem, fullOptions
)
assert fullCanonicalizer.getActiveParameter() == [0, 1, 2, 3, 4]
ott.assert_almost_equal(fullCanonicalizer.getCanonicalProblem().getDimension(), 6)
fullAggregate = fullCanonicalizer.getCanonicalProblem().getReliabilityMeasure()
parameter2Full = [2.0, 0.1, 3.0, 0.2, 0.0]
ott.assert_almost_equal(
    fullAggregate([1.0] + parameter2Full), aggregate([1.0]), 1.0e-6, 1.0e-6
)

# ------------------------------------------------- frozen parameter
options = otrobopt.CanonicalizationOptions()
options.setOptimizeParameter(False)
options.setParameterStart([4.0, 0.5])
canonicalizer = canonicalize(otrobopt.MeanMeasure(f, normal), options)
assert not canonicalizer.getOptimizeParameter()
assert canonicalizer.getReferenceDistribution() == ot.Normal(4.0, 0.5)
problem = canonicalizer.getCanonicalProblem()
ott.assert_almost_equal(problem.getDimension(), 1)
ott.assert_almost_equal(canonicalizer.getCanonicalStartingPoint(), [0.0])
canonicalMeasure = problem.getRobustnessMeasure()
ott.assert_almost_equal(canonicalMeasure([2.0]), [8.0])
ott.assert_almost_equal(canonicalMeasure([2.0]), otrobopt.MeanMeasure(f, ot.Normal(4.0, 0.5))([2.0]))

# No parameter start keeps the current distribution parameter
options = otrobopt.CanonicalizationOptions()
options.setOptimizeParameter(False)
canonicalizer = canonicalize(otrobopt.MeanMeasure(f, normal), options)
assert canonicalizer.getReferenceDistribution() == normal
ott.assert_almost_equal(canonicalizer.getCanonicalProblem().getRobustnessMeasure()([2.0]), [4.0])

# A parameter start of the wrong dimension is rejected
options = otrobopt.CanonicalizationOptions()
options.setParameterStart([1.0])
with ott.assert_raises(RuntimeError):
    canonicalize(otrobopt.MeanMeasure(f, normal), options)

# --------------------------------------------------- plain lifting
plainObjective = ot.SymbolicFunction(["x"], ["x^2"])
problem = otrobopt.RobustOptimizationProblem(plainObjective, otrobopt.MeanMeasure(f, normal))
canonicalizer = otrobopt.RobustOptimizationProblemCanonicalizer(problem)
canonicalObjective = canonicalizer.getCanonicalObjectiveFunction()
ott.assert_almost_equal(canonicalObjective.getInputDimension(), 3)
ott.assert_almost_equal(canonicalObjective([2.0, 2.0, 0.1]), [4.0])
ott.assert_almost_equal(
    canonicalizer.getCanonicalObjectiveFunction()(canonicalizer.getCanonicalStartingPoint()),
    [0.0],
)

baseG = ot.SymbolicFunction(["x", "theta"], ["theta - x"])
g = ot.ParametricFunction(baseG, [1], [2.0])
problem = otrobopt.RobustOptimizationProblem(otrobopt.MeanMeasure(f, normal), g)
canonicalizer = otrobopt.RobustOptimizationProblemCanonicalizer(problem)
canonicalG = canonicalizer.getCanonicalConstraintFunction()
ott.assert_almost_equal(canonicalG.getInputDimension(), 3)
ott.assert_almost_equal(canonicalG([1.0, 2.0, 0.1]), [1.0])

equality = ot.SymbolicFunction(["x"], ["x - 1.0"])
problem = otrobopt.RobustOptimizationProblem(otrobopt.MeanMeasure(f, normal), ot.Function())
problem.setEqualityConstraint(equality)
canonicalizer = otrobopt.RobustOptimizationProblemCanonicalizer(problem)
canonicalEquality = canonicalizer.getCanonicalProblem().getEqualityConstraint()
ott.assert_almost_equal(canonicalEquality.getInputDimension(), 3)
ott.assert_almost_equal(canonicalEquality([3.0, 2.0, 0.1]), [2.0])

# ------------------------------------------- bounds, start, types, direction
problem = otrobopt.RobustOptimizationProblem(otrobopt.MeanMeasure(f, normal), ot.Function())
problem.setBounds(ot.Interval([-1.0], [1.0]))
problem.setVariablesType([ot.OptimizationProblemImplementation.INTEGER])
problem.setMinimization(False)
options = otrobopt.CanonicalizationOptions()
options.setParameterBounds(ot.Interval([0.0, 0.0], [10.0, 10.0]))
options.setParameterStart([4.0, 0.5])
canonicalizer = otrobopt.RobustOptimizationProblemCanonicalizer(problem, options)
canonicalProblem = canonicalizer.getCanonicalProblem()
ott.assert_almost_equal(canonicalProblem.getBounds().getLowerBound(), [-1.0, 0.0, 0.0])
ott.assert_almost_equal(canonicalProblem.getBounds().getUpperBound(), [1.0, 10.0, 10.0])
ott.assert_almost_equal(canonicalizer.getCanonicalStartingPoint(), [0.0, 4.0, 0.5])
assert canonicalProblem.getVariablesType() == [
    ot.OptimizationProblemImplementation.INTEGER,
    ot.OptimizationProblemImplementation.CONTINUOUS,
    ot.OptimizationProblemImplementation.CONTINUOUS,
]
assert not canonicalProblem.isMinimization()

# The parameter can be bounded alone
problem = otrobopt.RobustOptimizationProblem(otrobopt.MeanMeasure(f, normal), ot.Function())
options = otrobopt.CanonicalizationOptions()
options.setParameterBounds(ot.Interval([0.0, 0.0], [10.0, 10.0]))
canonicalizer = otrobopt.RobustOptimizationProblemCanonicalizer(problem, options)
bounds = canonicalizer.getCanonicalProblem().getBounds()
ott.assert_almost_equal(bounds.getLowerBound(), [0.0, 0.0, 0.0])
ott.assert_almost_equal(bounds.getUpperBound(), [0.0, 10.0, 10.0])
assert not bounds.getFiniteLowerBound()[0]
assert not bounds.getFiniteUpperBound()[0]
assert bounds.getFiniteLowerBound()[1]
assert bounds.getFiniteUpperBound()[2]

# Frozen parameters keep the design bounds and variable types only
problem = otrobopt.RobustOptimizationProblem(otrobopt.MeanMeasure(f, normal), ot.Function())
problem.setBounds(ot.Interval([-1.0], [1.0]))
problem.setVariablesType([ot.OptimizationProblemImplementation.INTEGER])
options = otrobopt.CanonicalizationOptions()
options.setOptimizeParameter(False)
canonicalizer = otrobopt.RobustOptimizationProblemCanonicalizer(problem, options)
canonicalProblem = canonicalizer.getCanonicalProblem()
ott.assert_almost_equal(canonicalProblem.getBounds().getLowerBound(), [-1.0])
ott.assert_almost_equal(canonicalProblem.getBounds().getUpperBound(), [1.0])
assert canonicalProblem.getVariablesType() == [ot.OptimizationProblemImplementation.INTEGER]

# ------------------------------------------------------ worst case
baseWorst = ot.SymbolicFunction(["x", "theta"], ["x*theta"])
fWorst = ot.ParametricFunction(baseWorst, [1], [1.0])

# Standard fast path: a uniform has a bounded support
uniform = ot.Uniform(-1.0, 2.0)
worst = otrobopt.WorstCaseMeasure(fWorst, uniform, True)
canonicalizer = canonicalize(worst)
assert canonicalizer.usesStandardSpace()
canonicalWorst = canonicalizer.getCanonicalProblem().getRobustnessMeasure()
ott.assert_almost_equal(canonicalWorst([1.0, -1.0, 2.0]), [-1.0], 1.0e-3, 1.0e-3)
ott.assert_almost_equal(canonicalWorst([-1.0, -1.0, 2.0]), [-2.0], 1.0e-3, 1.0e-3)

# Maximization is preserved
worstMax = otrobopt.WorstCaseMeasure(fWorst, uniform, False)
canonicalizer = canonicalize(worstMax)
canonicalWorstMax = canonicalizer.getCanonicalProblem().getRobustnessMeasure()
ott.assert_almost_equal(canonicalWorstMax([1.0, -1.0, 2.0]), [2.0], 1.0e-3, 1.0e-3)

# A frozen rebuild preserves the worst-case domain stored by the measure
domainWorst = otrobopt.WorstCaseMeasure(fWorst, uniform, True)
domainWorst.setOptimizationDomain(ot.Interval([0.2], [0.4]))
options = otrobopt.CanonicalizationOptions()
options.setOptimizeParameter(False)
options.setParameterStart([0.0, 1.0])
domainCanonicalizer = canonicalize(domainWorst, options)
domainCanonicalWorst = domainCanonicalizer.getCanonicalProblem().getRobustnessMeasure()
ott.assert_almost_equal(domainCanonicalWorst([1.0]), [0.2], 1.0e-6, 1.0e-6)

# When the parameter is optimized the worst-case variable becomes the
# reference realization: a domain stored in the original theta coordinates
# cannot be transported, and the canonicalizer rejects it instead of silently
# dropping the restriction.
with ott.assert_raises(TypeError):
    canonicalize(domainWorst)

# A generic worst case fails closed without a domain
studentWorst = otrobopt.WorstCaseMeasure(fWorst, ot.Student(5.0, 0.0, 1.0), True)
with ott.assert_raises(TypeError):
    canonicalize(studentWorst)

# A bounded user reference is enough
constantWorst = otrobopt.WorstCaseMeasure(
    ot.ParametricFunction(
        ot.SymbolicFunction(["x", "theta"], ["x"]), [1], [0.0]
    ),
    ot.Student(5.0, 0.0, 1.0),
    True,
)
options = otrobopt.CanonicalizationOptions()
options.setReference(ot.Uniform(0.1, 0.9))
canonicalizer = canonicalize(constantWorst, options)
assert not canonicalizer.usesStandardSpace()
canonicalWorst = canonicalizer.getCanonicalProblem().getRobustnessMeasure()
ott.assert_almost_equal(canonicalWorst([2.0, 5.0, 0.0, 1.0]), [2.0], 1.0e-6, 1.0e-6)

# An explicit domain removes the need for a bounded reference
options = otrobopt.CanonicalizationOptions()
options.setWorstCasePolicy(otrobopt.CanonicalizationOptions.USER_DOMAIN)
options.setWorstCaseDomain(ot.Interval([-0.5], [0.5]))
canonicalizer = canonicalize(studentWorst, options)
canonicalWorst = canonicalizer.getCanonicalProblem().getRobustnessMeasure()
transformation = otrobopt.ParametricDistributionTransformation(
    ot.Student(5.0, 0.0, 1.0), ot.Normal(1)
)
expected = transformation([-0.5])[0]
ott.assert_almost_equal(canonicalWorst([1.0, 5.0, 0.0, 1.0]), [expected], 1.0e-4, 1.0e-4)

# NORMAL requires an explicit domain for the generic path
options = otrobopt.CanonicalizationOptions()
options.setWorstCasePolicy(otrobopt.CanonicalizationOptions.NORMAL)
with ott.assert_raises(TypeError):
    canonicalize(studentWorst, options)
options.setWorstCaseDomain(ot.Interval([-0.5], [0.5]))
canonicalizer = canonicalize(studentWorst, options)
assert canonicalizer.getReferenceDistribution() == ot.Normal(1)

# Explicit domain applied to the standard fast path
normalWorst = otrobopt.WorstCaseMeasure(fWorst, ot.Normal(2.0, 0.3), True)
options = otrobopt.CanonicalizationOptions()
options.setWorstCaseDomain(ot.Interval([-0.5], [0.5]))
canonicalizer = canonicalize(normalWorst, options)
assert canonicalizer.usesStandardSpace()
canonicalWorst = canonicalizer.getCanonicalProblem().getRobustnessMeasure()
ott.assert_almost_equal(canonicalWorst([1.0, 2.0, 0.3]), [1.85], 1.0e-4, 1.0e-4)

# A domain of the wrong dimension is rejected
options = otrobopt.CanonicalizationOptions()
options.setWorstCaseDomain(ot.Interval([-0.5, -0.5], [0.5, 0.5]))
with ott.assert_raises(RuntimeError):
    canonicalize(studentWorst, options)

# ------------------------------------------------ mixed distributions
otherNormal = ot.Normal([4.0, 5.0], [0.1, 0.2], ot.IdentityMatrix(2))
otherMean = otrobopt.MeanMeasure(f2, otherNormal)
with ott.assert_raises(RuntimeError):
    otrobopt.RobustOptimizationProblem(
        otrobopt.MeanMeasure(f, normal), otrobopt.MeanMeasure(f, otherNormal)
    )
mixedAggregate = otrobopt.AggregatedMeasure([mean2, otherMean])
problem = otrobopt.RobustOptimizationProblem(
    ot.SymbolicFunction(["x"], ["x"]), mixedAggregate
)
with ott.assert_raises(TypeError):
    otrobopt.RobustOptimizationProblemCanonicalizer(problem)

# ------------------------------------------------ invalid measures
with ott.assert_raises(RuntimeError):
    otrobopt.RobustOptimizationProblem(aggregate, ot.Function())
with ott.assert_raises(TypeError):
    problem = otrobopt.RobustOptimizationProblem(f2, ot.Function())
    problem.setObjective(ot.SymbolicFunction(["x"], ["x", "x^2"]))
with ott.assert_raises(TypeError):
    problem = otrobopt.RobustOptimizationProblem(ot.Function())
    problem.setRobustnessMeasure(aggregate)

# A problem without any measure is rejected
with ott.assert_raises(TypeError):
    otrobopt.RobustOptimizationProblemCanonicalizer(
        otrobopt.RobustOptimizationProblem(ot.Function(), ot.Function())
    )

# A measure with a zero-dimension input is rejected
with ott.assert_raises(RuntimeError):
    emptyMeasure = otrobopt.MeanMeasure(ot.Function(), ot.Normal(1))
    otrobopt.RobustOptimizationProblemCanonicalizer(
        otrobopt.RobustOptimizationProblem(emptyMeasure, ot.Function())
    )
