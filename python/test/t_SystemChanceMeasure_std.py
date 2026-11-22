#!/usr/bin/env python

import math
import os
import tempfile

import openturns as ot
import openturns.testing as ott
import otrobopt

ot.RandomGenerator.SetSeed(12345)

# ------------------------------------------------------------ elementary case
limit_state = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta"], ["x - theta"]), [1], [0.0]
)
identity = ot.SymbolicFunction(["y0"], ["y0"])
alpha = 0.9
alpha_probit = ot.Normal().computeQuantile(alpha)[0]
half = ot.Normal().computeCDF(0.0)

measure = otrobopt.SystemChanceMeasure(
    limit_state, identity, ot.Normal(), ot.GreaterOrEqual(), alpha
)
assert repr(measure).count("QuadratureEstimator") == 1
assert measure.getOutputDimension() == 1
assert measure.getAlpha() == alpha
assert str(measure.getOperator()) == ">="

probability = measure.getSafeProbability([0.0])
ott.assert_almost_equal(probability, half, 1e-12, 1e-12)
ott.assert_almost_equal(measure.getReliabilityIndex([0.0]), 0.0, 1e-12, 1e-12)
ott.assert_almost_equal(measure([0.0])[0], -alpha_probit, 1e-12, 1e-12)
assert math.isnan(measure.getEstimationError([0.0]))

# The operator only orients the constraint: alpha is the target safe
# probability, so a Less operator returns the opposite sign.
less_measure = otrobopt.SystemChanceMeasure(
    limit_state, identity, ot.Normal(), ot.Less(), alpha
)
ott.assert_almost_equal(less_measure([0.0])[0], alpha_probit, 1e-12, 1e-12)

# ------------------------------------------------ experiment estimator (D4)
# The integrand is the indicator only.  The historical density-weighted
# convention returns qNormal(E[p(Theta) 1_E]) - qNormal(alpha) = -2.35746 on
# this case; the correct probit is -1.28155.
sample_size = 200000
measure.setEstimator(otrobopt.ExperimentEstimator(ot.MonteCarloExperiment(sample_size)))
assert "ExperimentEstimator" in repr(measure.getEstimator())
sample_probability = measure.getSafeProbability([0.0])
sample_error = measure.getEstimationError([0.0])
assert 0.0 < sample_error < 0.01
assert abs(sample_probability - half) <= 4.0 * sample_error
sample_value = measure([0.0])[0]
assert abs(sample_value + 1.28155) < 0.01
assert abs(sample_value + 2.35746) > 0.5
# The estimator is stateless by contract: every call draws a new sample.  The
# index must still be exactly the probit of the probability of the same draw.
generator_state = ot.RandomGenerator.GetState()
index = measure.getReliabilityIndex([0.0])
ot.RandomGenerator.SetState(generator_state)
same_draw_probability = measure.getSafeProbability([0.0])
ott.assert_almost_equal(
    index,
    ot.Normal().computeQuantile(same_draw_probability)[0],
    1e-12,
    1e-12,
)

# --------------------------------------- D4: all leaves against the reference
full = ot.SymbolicFunction(
    ["x0", "x1", "theta0", "theta1"], ["x0 - theta0", "x1 - theta1"]
)
limit_state_2 = ot.ParametricFunction(full, [2, 3], [0.0, 0.0])
parameter_distribution_2 = ot.Normal(2)
all_leaves = ot.SymbolicFunction(["y0", "y1"], ["y0 * y1"])

system = otrobopt.SystemChanceMeasure(
    limit_state_2, all_leaves, parameter_distribution_2, ot.GreaterOrEqual(), alpha
)
system.setEstimator(otrobopt.QuadratureEstimator())
reference = otrobopt.JointChanceMeasure(
    limit_state_2, parameter_distribution_2, ot.GreaterOrEqual(), alpha
)

for design_point in ([0.6, -0.4], [0.0, 0.0], [1.0, 1.0]):
    system_value = system(design_point)[0]
    reference_value = reference(design_point)[0]
    ott.assert_almost_equal(system_value, reference_value, 1e-12, 1e-12)
    # Invert both probits to compare the probabilities themselves.
    system_probability = system.getSafeProbability(design_point)
    reference_probability = ot.Normal().computeCDF(reference_value + alpha_probit)
    ott.assert_almost_equal(system_probability, reference_probability, 1e-12, 1e-12)

# --------------------------------------------- event predicate is inspected
for predicate, expected in [
    (ot.SymbolicFunction(["y0", "y1"], ["y0 + y1 - y0 * y1"]), 1.0 - (1.0 - half) ** 2),
    (ot.SymbolicFunction(["y0", "y1"], ["y0 + y1 - 2 * y0 * y1"]), 2 * half * (1 - half)),
]:
    event_measure = otrobopt.SystemChanceMeasure(
        limit_state_2, predicate, parameter_distribution_2, ot.GreaterOrEqual(), alpha
    )
    ott.assert_almost_equal(
        event_measure.getSafeProbability([0.0, 0.0]), expected, 1e-7, 1e-7
    )

# ------------------------------------------------------------ discretization
# The safe atom is below SpecFunc-Precision.  The exact discrete sum keeps it,
# so the measure must return a finite probit rather than a clipped +0 or +1.
support = ot.Sample([[0.0], [2.0]])
discrete = ot.FiniteDiscreteDistribution(support, [1.0e-17, 1.0])
discrete_measure = otrobopt.SystemChanceMeasure(
    limit_state, identity, discrete, ot.GreaterOrEqual(), alpha
)
ott.assert_almost_equal(
    discrete_measure.getSafeProbability([1.0]), 1.0e-17, 1e-25, 1e-25
)
assert math.isfinite(discrete_measure([1.0])[0])

# --------------------------------------------------------------- rebuild
# rebuild() returns the generic measure handle, so only its value is checked.
# The estimator itself is a shared handle copied by rebuild(); its survival
# through the storage layer is covered by the persistence block below.
shifted = system.rebuild(limit_state_2, ot.Normal([0.5, -0.5], [1.0, 1.0]))
shifted_probability = ot.Normal().computeCDF(-0.5) * ot.Normal().computeCDF(0.5)
ott.assert_almost_equal(
    shifted([0.0, 0.0])[0],
    ot.Normal().computeQuantile(shifted_probability)[0] - alpha_probit,
    1e-10,
    1e-10,
)

# -------------------------------------------------------------- validation
with ott.assert_raises(Exception):
    otrobopt.SystemChanceMeasure(
        limit_state, ot.SymbolicFunction(["y0", "y1"], ["y0"]), ot.Normal(), ot.Greater(), 0.9
    )
with ott.assert_raises(Exception):
    otrobopt.SystemChanceMeasure(
        limit_state,
        ot.SymbolicFunction(["y0"], ["y0", "y0"]),
        ot.Normal(),
        ot.Greater(),
        0.9,
    )
with ott.assert_raises(Exception):
    otrobopt.SystemChanceMeasure(
        limit_state,
        ot.SymbolicFunction(["y0"], ["2 + y0"]),
        ot.Normal(),
        ot.Greater(),
        0.9,
    )
with ott.assert_raises(Exception):
    otrobopt.SystemChanceMeasure(
        limit_state, identity, ot.Normal(), ot.Greater(), 0.0
    )
with ott.assert_raises(Exception):
    otrobopt.SystemChanceMeasure(
        limit_state, identity, ot.Normal(), ot.Greater(), 1.0
    )
with ott.assert_raises(Exception):
    measure([0.0, 1.0])

# ------------------------------------------------------------ persistence
handle, file_name = tempfile.mkstemp(suffix=".xml")
os.close(handle)
try:
    study = ot.Study()
    study.setStorageManager(ot.XMLStorageManager(file_name))
    study.add("system", measure)
    study.save()

    study = ot.Study()
    study.setStorageManager(ot.XMLStorageManager(file_name))
    study.load()
    reloaded = otrobopt.SystemChanceMeasure()
    study.fillObject("system", reloaded)
    reloaded_estimator = repr(reloaded.getEstimator())
    assert "ExperimentEstimator" in reloaded_estimator
    assert "size=200000" in reloaded_estimator
    # The experiment resamples at every call: check the restored estimator
    # statistically, not against the draw saved before the round trip.
    reloaded_probability = reloaded.getSafeProbability([0.0])
    reloaded_error = reloaded.getEstimationError([0.0])
    assert 0.0 < reloaded_error < 0.01
    assert abs(reloaded_probability - half) <= 4.0 * reloaded_error
    assert reloaded.getEventPredicate().getInputDimension() == 1
    assert reloaded.getAlpha() == alpha
finally:
    os.remove(file_name)
