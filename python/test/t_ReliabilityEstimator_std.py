#!/usr/bin/env python

import math
import os
import tempfile

import openturns as ot
import openturns.testing as ott
import otrobopt

ot.RandomGenerator.SetSeed(12345)


def check_raises(function, *args):
    # ott.assert_raises does not detect a missing exception in this version.
    try:
        function(*args)
    except Exception:
        return
    raise AssertionError("an exception was expected from " + function.__name__)


normal = ot.Normal()
identity = ot.SymbolicFunction(["y0"], ["y0"])
limit_state = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta"], ["x - theta"]), [1], [0.0]
)

# ------------------------------------------------------------ one-component FORM
estimator = otrobopt.ReliabilityEstimator("FORM", "ANALYTICAL")
assert "FORM" in repr(estimator)
assert "ANALYTICAL" in repr(estimator)
assert estimator.getComponentAlgorithm() == "FORM"
assert estimator.getSystemAlgorithm() == "ANALYTICAL"
assert estimator.hasGeometry()

probability = estimator.getSafeProbability(identity, limit_state, normal, [1.0])
# The limit state is linear and the standard space is normal: FORM is exact.
ott.assert_almost_equal(probability, normal.computeCDF(1.0), 1e-6, 1e-6)
ott.assert_almost_equal(
    estimator.getComponentReliabilityIndexes(identity, limit_state, normal, [1.0])[0],
    1.0,
    1e-6,
    1e-6,
)
assert math.isnan(estimator.getEstimationError(identity, limit_state, normal, [1.0]))
bounds = estimator.getProbabilityBounds(identity, limit_state, normal, [1.0])
ott.assert_almost_equal(bounds.getLowerBound()[0], probability, 1e-12, 1e-12)
ott.assert_almost_equal(bounds.getUpperBound()[0], probability, 1e-12, 1e-12)
design_points = estimator.getStandardDesignPoints(identity, limit_state, normal, [1.0])
assert design_points.getSize() == 1
ott.assert_almost_equal(design_points[0][0], 1.0, 1e-6, 1e-6)
assert list(estimator.getDesignPointsNumber(identity, limit_state, normal, [1.0])) == [1]

# The operator only orients the target: an unsafe origin gives a negative
# index and the complementary probability.
negative_limit_state = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta"], ["theta - x"]), [1], [0.0]
)
negative_probability = estimator.getSafeProbability(
    identity, negative_limit_state, normal, [1.0]
)
ott.assert_almost_equal(negative_probability, normal.computeCDF(-1.0), 1e-6, 1e-6)
ott.assert_almost_equal(
    estimator.getComponentReliabilityIndexes(
        identity, negative_limit_state, normal, [1.0]
    )[0],
    -1.0,
    1e-6,
    1e-6,
)

# ------------------------------------------------ two independent components
full = ot.SymbolicFunction(
    ["x0", "x1", "theta0", "theta1"], ["x0 - theta0", "x1 - theta1"]
)
limit_state_2 = ot.ParametricFunction(full, [2, 3], [0.0, 0.0])
all_leaves = ot.SymbolicFunction(["y0", "y1"], ["y0 * y1"])
any_leaf = ot.SymbolicFunction(["y0", "y1"], ["y0 + y1 - y0 * y1"])
normal_2 = ot.Normal(2)
component_probability = normal.computeCDF(1.0)
series_probability = estimator.getSafeProbability(
    all_leaves, limit_state_2, normal_2, [1.0, 1.0]
)
ott.assert_almost_equal(
    series_probability, component_probability**2, 1e-6, 1e-6
)
parallel_probability = estimator.getSafeProbability(
    any_leaf, limit_state_2, normal_2, [1.0, 1.0]
)
ott.assert_almost_equal(
    parallel_probability, 1.0 - (1.0 - component_probability) ** 2, 1e-6, 1e-6
)

# ------------------------------------------------- correlated linear components
correlation = ot.CorrelationMatrix(2)
correlation[1, 0] = 0.5
normal_2_correlated = ot.Normal([0.0, 0.0], [1.0, 1.0], correlation)
correlated_probability = estimator.getSafeProbability(
    all_leaves, limit_state_2, normal_2_correlated, [1.0, 1.0]
)
ott.assert_almost_equal(
    correlated_probability,
    normal_2_correlated.computeCDF([1.0, 1.0]),
    1e-6,
    1e-6,
)

# --------------------------------------------------------------- MultiFORM
# The failure domain |theta| > sqrt(x) has two symmetric design points.  FORM
# keeps one of them, MULTIFORM keeps both and recovers the exact probability.
quadratic_limit_state = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta"], ["x - theta * theta"]), [1], [0.0]
)
multi_form = otrobopt.ReliabilityEstimator("MULTIFORM", "ANALYTICAL")
# Both component algorithms are checked through the explicit setters.
multi_form.setComponentAlgorithm("FORM")
form_only_probability = multi_form.getSafeProbability(
    identity, quadratic_limit_state, normal, [1.0]
)
ott.assert_almost_equal(form_only_probability, normal.computeCDF(1.0), 1e-6, 1e-6)
assert list(multi_form.getDesignPointsNumber(identity, quadratic_limit_state, normal, [1.0])) == [1]
multi_form.setComponentAlgorithm("MULTIFORM")
ott.assert_almost_equal(
    multi_form.getSafeProbability(identity, quadratic_limit_state, normal, [1.0]),
    2.0 * normal.computeCDF(1.0) - 1.0,
    1e-6,
    1e-6,
)
assert list(multi_form.getDesignPointsNumber(identity, quadratic_limit_state, normal, [1.0])) == [2]
assert multi_form.getStandardDesignPoints(
    identity, quadratic_limit_state, normal, [1.0]
).getSize() == 2
ott.assert_almost_equal(
    multi_form.getComponentReliabilityIndexes(
        identity, quadratic_limit_state, normal, [1.0]
    )[0],
    normal.computeQuantile(2.0 * normal.computeCDF(1.0) - 1.0)[0],
    1e-6,
    1e-6,
)

# An unsafe origin flips the sign of the index.  MultiFORM linearizes the
# failure event as the union of the tangent failure half-spaces; on this
# concave failure domain that union covers the whole line, so the model value
# is zero and the index stays negative instead of becoming positive.
unsafe_limit_state = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta"], ["theta * theta - x"]), [1], [1.0]
)
unsafe = otrobopt.ReliabilityEstimator("FORM", "ANALYTICAL")
ott.assert_almost_equal(
    unsafe.getSafeProbability(identity, unsafe_limit_state, normal, [1.0]),
    normal.computeCDF(-1.0),
    1e-6,
    1e-6,
)
unsafe.setComponentAlgorithm("MULTIFORM")
ott.assert_almost_equal(
    unsafe.getSafeProbability(identity, unsafe_limit_state, normal, [1.0]),
    0.0,
    1e-12,
    1e-12,
)
assert unsafe.getComponentReliabilityIndexes(
    identity, unsafe_limit_state, normal, [1.0]
)[0] < -5.0
assert list(
    unsafe.getDesignPointsNumber(identity, unsafe_limit_state, normal, [1.0])
) == [2]

# ------------------------------------------------------- bounded combination
# Two components with two design points each: the exact combination needs
# 2^4 atoms, the first-order bounds only 2^2.  A maximum of 10 forces the
# bounded path, whose interval must contain the reference probability.
bounded_state = ot.ParametricFunction(
    ot.SymbolicFunction(
        ["x0", "x1", "theta0", "theta1"],
        ["x0 - theta0 * theta0", "x1 - theta1 * theta1"],
    ),
    [2, 3],
    [0.0, 0.0],
)
bounded = otrobopt.ReliabilityEstimator("MULTIFORM", "ANALYTICAL")
bounded.setMaximumDesignPointsNumber(2)
bounded.setMaximumCombinationNumber(10)
reference_probability = (2.0 * normal.computeCDF(1.0) - 1.0) ** 2
bounded_probability = bounded.getSafeProbability(
    all_leaves, bounded_state, normal_2, [1.0, 1.0]
)
bounded_error = bounded.getEstimationError(
    all_leaves, bounded_state, normal_2, [1.0, 1.0]
)
bounded_bounds = bounded.getProbabilityBounds(
    all_leaves, bounded_state, normal_2, [1.0, 1.0]
)
assert bounded_bounds.getLowerBound()[0] <= reference_probability <= bounded_bounds.getUpperBound()[0]
ott.assert_almost_equal(
    bounded_probability,
    0.5 * (bounded_bounds.getLowerBound()[0] + bounded_bounds.getUpperBound()[0]),
    1e-8,
    1e-8,
)
ott.assert_almost_equal(
    bounded_error,
    0.5 * (bounded_bounds.getUpperBound()[0] - bounded_bounds.getLowerBound()[0]),
    1e-8,
    1e-8,
)
assert list(
    bounded.getDesignPointsNumber(all_leaves, bounded_state, normal_2, [1.0, 1.0])
) == [2, 2]

# Both the exact sum and the bounds need more atoms than allowed.
too_small = otrobopt.ReliabilityEstimator("MULTIFORM", "ANALYTICAL")
too_small.setMaximumDesignPointsNumber(2)
too_small.setMaximumCombinationNumber(3)
check_raises(too_small.getSafeProbability, all_leaves, bounded_state, normal_2, [1.0, 1.0])

# ------------------------------------------------------- simulation fallback
# The analytical combination requires a normal standard space; other
# distributions are handled by sampling and cannot provide a geometry.
simulation = otrobopt.ReliabilityEstimator("FORM", "SIMULATION")
assert simulation.getSystemAlgorithm() == "SIMULATION"
assert not simulation.hasGeometry()
simulation.setSampleSize(200000)
student = ot.Student(5.0)
student_probability = simulation.getSafeProbability(
    identity, limit_state, student, [1.0]
)
student_error = simulation.getEstimationError(identity, limit_state, student, [1.0])
assert 0.0 < student_error < 0.01
assert abs(student_probability - student.computeCDF(1.0)) <= 4.0 * student_error
check_raises(estimator.getSafeProbability, identity, limit_state, student, [1.0])
check_raises(
    simulation.getComponentReliabilityIndexes, identity, limit_state, student, [1.0]
)
check_raises(simulation.getStandardDesignPoints, identity, limit_state, student, [1.0])
check_raises(simulation.getDesignPointsNumber, identity, limit_state, student, [1.0])
check_raises(simulation.getProbabilityBounds, identity, limit_state, student, [1.0])

# ------------------------------------------------------------------ options
for component_algorithm, system_algorithm in (
    ("form", "ANALYTICAL"),
    ("MULTIFORM2", "ANALYTICAL"),
    ("FORM", "analytical"),
    ("FORM", "SAMPLING"),
):
    check_raises(
        otrobopt.ReliabilityEstimator, component_algorithm, system_algorithm
    )
for setter, value in (
    (estimator.setMaximumDesignPointsNumber, 0),
    (estimator.setMaximumCombinationNumber, 0),
    (estimator.setBetaTarget, 0.0),
    (estimator.setBetaTarget, -1.0),
    (estimator.setSampleSize, 0),
):
    check_raises(setter, value)
check_raises(
    estimator.getSafeProbability,
    ot.SymbolicFunction(["y0"], ["2 + y0"]),
    limit_state,
    normal,
    [1.0],
)
check_raises(estimator.getSafeProbability, identity, limit_state, normal, [0.0, 1.0])

# ------------------------------------------------------------ persistence
handle, file_name = tempfile.mkstemp(suffix=".xml")
os.close(handle)
persistent = otrobopt.ReliabilityEstimator("MULTIFORM", "SIMULATION")
persistent.setMaximumDesignPointsNumber(3)
persistent.setMaximumCombinationNumber(64)
persistent.setBetaTarget(1.5)
persistent.setSampleSize(1000)
try:
    study = ot.Study()
    study.setStorageManager(ot.XMLStorageManager(file_name))
    study.add("estimator", persistent)
    study.save()

    study = ot.Study()
    study.setStorageManager(ot.XMLStorageManager(file_name))
    study.load()
    reloaded = otrobopt.ReliabilityEstimator()
    study.fillObject("estimator", reloaded)
    assert reloaded.getComponentAlgorithm() == "MULTIFORM"
    assert reloaded.getSystemAlgorithm() == "SIMULATION"
    assert reloaded.getMaximumDesignPointsNumber() == 3
    assert reloaded.getMaximumCombinationNumber() == 64
    assert reloaded.getBetaTarget() == 1.5
    assert reloaded.getSampleSize() == 1000
    assert "Cobyla" in repr(reloaded.getSolver())
    ot.RandomGenerator.SetSeed(12345)
    reloaded_probability = reloaded.getSafeProbability(
        identity, limit_state, normal, [1.0]
    )
    assert 0.0 < reloaded_probability < 1.0
    assert reloaded.getEstimationError(identity, limit_state, normal, [1.0]) > 0.0
finally:
    os.remove(file_name)
