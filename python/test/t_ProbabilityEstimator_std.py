#!/usr/bin/env python

import math
import os
import tempfile

import openturns as ot
import openturns.testing as ott
import otrobopt

ot.RandomGenerator.SetSeed(12345)

# --------------------------------------------------------------- one leaf
limit_state = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta"], ["x - theta"]), [1], [0.0]
)
identity = ot.SymbolicFunction(["y0"], ["y0"])
theta_dist = ot.Normal()
quadrature = otrobopt.QuadratureEstimator()
experiment = otrobopt.ExperimentEstimator(ot.MonteCarloExperiment(200000))

assert not quadrature.hasGeometry()
assert not experiment.hasGeometry()
assert "QuadratureEstimator" in repr(quadrature)
assert "ExperimentEstimator" in repr(experiment)
assert "MonteCarloExperiment" in repr(experiment)

for design_point in [-1.5, 0.0, 1.0]:
    expected = ot.Normal().computeCDF(design_point)
    probability = quadrature.getSafeProbability(
        identity, limit_state, theta_dist, [design_point]
    )
    ott.assert_almost_equal(probability, expected, 1e-10, 1e-10)
    assert math.isnan(
        quadrature.getEstimationError(
            identity, limit_state, theta_dist, [design_point]
        )
    )

    sample_probability = experiment.getSafeProbability(
        identity, limit_state, theta_dist, [design_point]
    )
    error = experiment.getEstimationError(
        identity, limit_state, theta_dist, [design_point]
    )
    assert 0.0 < error < 0.01
    assert abs(sample_probability - expected) <= 4.0 * error + 1e-12

ott.assert_almost_equal(
    quadrature.getSafeProbability(identity, limit_state, theta_dist, [0.0]),
    0.5,
    1e-12,
    1e-12,
)

# ------------------------------------------- two leaves, independent inputs
full = ot.SymbolicFunction(
    ["x0", "x1", "theta0", "theta1"],
    ["x0 - theta0", "x1 - theta1"],
)
limit_state_2 = ot.ParametricFunction(full, [2, 3], [0.0, 0.0])
theta_dist_2 = ot.Normal(2)
all_leaves = ot.SymbolicFunction(["y0", "y1"], ["y0 * y1"])
any_leaf = ot.SymbolicFunction(["y0", "y1"], ["y0 + y1 - y0 * y1"])
exclusive = ot.SymbolicFunction(["y0", "y1"], ["y0 + y1 - 2 * y0 * y1"])
one = ot.SymbolicFunction(["y0", "y1"], ["1"])
zero = ot.SymbolicFunction(["y0", "y1"], ["0"])

design_point_2 = [0.6, -0.4]
p0 = ot.Normal().computeCDF(design_point_2[0])
p1 = ot.Normal().computeCDF(design_point_2[1])
expected = {
    "all": p0 * p1,
    "any": 1.0 - (1.0 - p0) * (1.0 - p1),
    "exclusive": p0 * (1.0 - p1) + (1.0 - p0) * p1,
}
for predicate, key in [
    (all_leaves, "all"),
    (any_leaf, "any"),
    (exclusive, "exclusive"),
]:
    value = quadrature.getSafeProbability(
        predicate, limit_state_2, theta_dist_2, design_point_2
    )
    # Iterated Gauss-Kronrod on a 2-D product: measured gap is about 4e-9.
    ott.assert_almost_equal(value, expected[key], 1e-7, 1e-7)

ott.assert_almost_equal(
    quadrature.getSafeProbability(one, limit_state_2, theta_dist_2, design_point_2),
    1.0,
)
ott.assert_almost_equal(
    quadrature.getSafeProbability(zero, limit_state_2, theta_dist_2, design_point_2),
    0.0,
)

# ------------------------------------------------------- discrete exactness
# The safe atom is below SpecFunc::Precision.  The historical joint measure
# drops it with its density threshold; the exact discrete sum must keep it.
support = ot.Sample([[0.0], [2.0]])
discrete = ot.FiniteDiscreteDistribution(support, [1.0e-17, 1.0])
ott.assert_almost_equal(
    quadrature.getSafeProbability(identity, limit_state, discrete, [1.0]),
    1.0e-17,
    1e-25,
    1e-25,
)
complement = ot.SymbolicFunction(["y0"], ["1 - y0"])
ott.assert_almost_equal(
    quadrature.getSafeProbability(complement, limit_state, discrete, [1.0]),
    1.0,
    1e-15,
    1e-15,
)

# -------------------------------------------------------------- validation
with ott.assert_raises(Exception):
    quadrature.getSafeProbability(identity, limit_state, theta_dist, [0.0, 1.0])
with ott.assert_raises(Exception):
    quadrature.getSafeProbability(identity, limit_state_2, theta_dist, [0.0, 0.0])
with ott.assert_raises(Exception):
    quadrature.getSafeProbability(
        ot.SymbolicFunction(["y0", "y1"], ["y0"]), limit_state, theta_dist, [0.0]
    )
with ott.assert_raises(Exception):
    quadrature.getSafeProbability(
        ot.SymbolicFunction(["y0"], ["y0", "y0"]), limit_state, theta_dist, [0.0]
    )
with ott.assert_raises(Exception):
    quadrature.getSafeProbability(identity, limit_state, ot.Normal(2), [0.0])
with ott.assert_raises(Exception):
    parametric_predicate = ot.ParametricFunction(
        ot.SymbolicFunction(["y0", "p"], ["y0 - p"]), [1], [0.0]
    )
    quadrature.getSafeProbability(
        parametric_predicate, limit_state, theta_dist, [0.0]
    )
with ott.assert_raises(Exception):
    # Non-Boolean at the zero probe
    quadrature.getSafeProbability(
        ot.SymbolicFunction(["y0"], ["2 + y0"]), limit_state, theta_dist, [0.0]
    )
with ott.assert_raises(Exception):
    # Boolean at the zero probe, non-Boolean on the integration nodes
    quadrature.getSafeProbability(
        ot.SymbolicFunction(["y0"], ["0.5 * y0"]), limit_state, theta_dist, [0.0]
    )

# ------------------------------------------------------------ persistence
handle, file_name = tempfile.mkstemp(suffix=".xml")
os.close(handle)
try:
    study = ot.Study()
    study.setStorageManager(ot.XMLStorageManager(file_name))
    study.add("quadrature", quadrature)
    study.add("experiment", experiment)
    study.save()

    study = ot.Study()
    study.setStorageManager(ot.XMLStorageManager(file_name))
    study.load()
    quadrature_2 = otrobopt.QuadratureEstimator()
    study.fillObject("quadrature", quadrature_2)
    experiment_2 = otrobopt.ExperimentEstimator()
    study.fillObject("experiment", experiment_2)
    ott.assert_almost_equal(
        quadrature_2.getSafeProbability(identity, limit_state, theta_dist, [0.0]),
        0.5,
        1e-10,
        1e-10,
    )
    assert experiment_2.getExperiment().getSize() == 200000
finally:
    os.remove(file_name)
