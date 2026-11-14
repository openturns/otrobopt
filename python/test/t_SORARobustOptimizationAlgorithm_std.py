#!/usr/bin/env python

import math
import os
import tempfile

import openturns as ot
import openturns.testing as ott
import otrobopt


def make_problem(operator=ot.GreaterOrEqual(), alpha=0.9):
    limit_state = ot.ParametricFunction(
        ot.SymbolicFunction(["x", "theta"], ["theta - x"]), [1], [0.0]
    )
    measure = otrobopt.JointChanceMeasure(
        limit_state, ot.Normal(), operator, alpha
    )
    return otrobopt.RBDOProblem(
        ot.SymbolicFunction(["x"], ["x^2"]), measure
    )


def make_problem_2d(alpha=0.9):
    limit_state = ot.ParametricFunction(
        ot.SymbolicFunction(
            ["x1", "x2", "t1", "t2"],
            ["t1 - x1 - x2", "t2 - x1 - x2"],
        ),
        [2, 3],
        [0.0, 0.0],
    )
    distribution = ot.JointDistribution([ot.Normal(), ot.Normal()])
    measure = otrobopt.JointChanceMeasure(
        limit_state, distribution, ot.GreaterOrEqual(), alpha
    )
    return otrobopt.RBDOProblem(
        ot.SymbolicFunction(["x1", "x2"], ["x1^2 + x2^2"]), measure
    )


# --------------------------------------------------------- empty algorithm
empty = otrobopt.SORARobustOptimizationAlgorithm()
assert "SORARobustOptimizationAlgorithm" in repr(empty)
assert empty.getJointChanceTreatment() == otrobopt.SORARobustOptimizationAlgorithm.EXACT
assert "EXACT" in repr(empty)
with ott.assert_raises(Exception):
    empty.run()
with ott.assert_raises(Exception):
    empty.getReliabilityIndex()
with ott.assert_raises(Exception):
    empty.getPerformanceMeasure()
with ott.assert_raises(Exception):
    empty.getShift()
with ott.assert_raises(Exception):
    empty.getStandardSpaceDesignPoint()
with ott.assert_raises(Exception):
    empty.getPhysicalSpaceDesignPoint()
with ott.assert_raises(Exception):
    empty.getApproximateProbability()

plain = ot.OptimizationProblem()
plain.setObjective(ot.SymbolicFunction(["x"], ["x^2"]))
with ott.assert_raises(Exception):
    empty.setProblem(plain)

# --------------------------------------------- target probability contract
problem = make_problem(alpha=0.9)
algo = otrobopt.SORARobustOptimizationAlgorithm(problem, ot.Cobyla())
assert algo.getRBDOProblem().getTargetProbability() == 0.9
with ott.assert_raises(Exception):
    otrobopt.SORARobustOptimizationAlgorithm(make_problem(alpha=0.5), ot.Cobyla())
with ott.assert_raises(Exception):
    otrobopt.SORARobustOptimizationAlgorithm(make_problem(alpha=0.4), ot.Cobyla())
# For a Less operator the safe-event probability is 1 - alpha.
with ott.assert_raises(Exception):
    otrobopt.SORARobustOptimizationAlgorithm(
        make_problem(ot.Less(), 0.6), ot.Cobyla()
    )
otrobopt.SORARobustOptimizationAlgorithm(make_problem(ot.Less(), 0.4), ot.Cobyla())

# --------------------------------- physical shift dimension requirement
limit_state = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "t1", "t2"], ["t1 - x", "t2 - x"]),
    [1, 2],
    [0.0, 0.0],
)
measure = otrobopt.JointChanceMeasure(
    limit_state,
    ot.JointDistribution([ot.Normal(), ot.Normal()]),
    ot.GreaterOrEqual(),
    0.9,
)
mismatched = otrobopt.RBDOProblem(
    ot.SymbolicFunction(["x"], ["x^2"]), measure
)
try:
    otrobopt.SORARobustOptimizationAlgorithm(mismatched, ot.Cobyla())
    raise AssertionError("The dimension mismatch was not rejected")
except RuntimeError as exc:
    assert "physical shift" in str(exc)
    assert "dimension" in str(exc)

# ------------------------------------------------- 1D Greater than target
expected = ot.Normal().computeQuantile(0.1)[0]
target_index = ot.DistFunc.qNormal(0.9)
algo.setStartingPoint([0.0])
algo.run()
result = algo.getResult()
ott.assert_almost_equal(result.getOptimalPoint(), [expected], 5e-3, 5e-3)
ott.assert_almost_equal(
    result.getOptimalValue(), [expected**2], 1e-2, 1e-2
)
assert result.getStatus() == ot.OptimizationResult.SUCCESS
assert "EXACT" in result.getStatusMessage()
ott.assert_almost_equal(algo.getTargetReliabilityIndex(), target_index)
ott.assert_almost_equal(
    algo.getReliabilityIndex(), target_index, 1e-2, 1e-2
)
ott.assert_almost_equal(
    algo.getApproximateProbability(), 0.9, 1e-2, 1e-2
)
ott.assert_almost_equal(
    algo.getStandardSpaceDesignPoint(), [expected], 5e-3, 5e-3
)
ott.assert_almost_equal(
    algo.getPhysicalSpaceDesignPoint(), [expected], 5e-3, 5e-3
)
ott.assert_almost_equal(algo.getPerformanceMeasure(), 0.0, 1e-2, 1e-2)
ott.assert_almost_equal(algo.getShift(), [0.0], 1e-2, 1e-2)

# ----------------------------------------------------- 1D Less than target
problem_less = make_problem(ot.Less(), 0.1)
expected_less = ot.Normal().computeQuantile(0.9)[0]
algo_less = otrobopt.SORARobustOptimizationAlgorithm(problem_less, ot.Cobyla())
algo_less.setStartingPoint([0.0])
algo_less.run()
ott.assert_almost_equal(
    algo_less.getResult().getOptimalPoint(), [expected_less], 5e-3, 5e-3
)
ott.assert_almost_equal(
    algo_less.getReliabilityIndex(), target_index, 1e-2, 1e-2
)
ott.assert_almost_equal(
    algo_less.getPhysicalSpaceDesignPoint(), [expected_less], 5e-3, 5e-3
)

# ------------------------------------------------ 2D joint chance modes
problem_2d = make_problem_2d(0.9)
algo_2d = otrobopt.SORARobustOptimizationAlgorithm(problem_2d, ot.Cobyla())
algo_2d.setStartingPoint([0.0, 0.0])

# Exact joint event: one scalarized limit state with the target index.
algo_2d.setJointChanceTreatment(
    otrobopt.SORARobustOptimizationAlgorithm.EXACT
)
assert (
    algo_2d.getJointChanceTreatment()
    == otrobopt.SORARobustOptimizationAlgorithm.EXACT
)
algo_2d.run()
ott.assert_almost_equal(
    algo_2d.getResult().getOptimalPoint(),
    [-target_index / 2.0, -target_index / 2.0],
    5e-3,
    5e-3,
)
ott.assert_almost_equal(algo_2d.getTargetReliabilityIndex(), target_index)
ott.assert_almost_equal(
    algo_2d.getReliabilityIndex(), target_index, 1e-2, 1e-2
)
ott.assert_almost_equal(
    algo_2d.getShift().norm(), target_index / math.sqrt(2.0), 1e-2, 1e-2
)

# Bonferroni: the component target is 1 - (1 - alpha) / n.
algo_2d.setJointChanceTreatment(
    otrobopt.SORARobustOptimizationAlgorithm.BONFERRONI
)
assert (
    algo_2d.getJointChanceTreatment()
    == otrobopt.SORARobustOptimizationAlgorithm.BONFERRONI
)
assert "BONFERRONI" in repr(algo_2d)
bonferroni_index = ot.DistFunc.qNormal(0.95)
ott.assert_almost_equal(
    algo_2d.getTargetReliabilityIndex(), bonferroni_index
)
algo_2d.run()
ott.assert_almost_equal(
    algo_2d.getResult().getOptimalPoint(),
    [-bonferroni_index / 2.0, -bonferroni_index / 2.0],
    5e-3,
    5e-3,
)
ott.assert_almost_equal(
    algo_2d.getReliabilityIndex(), bonferroni_index, 1e-2, 1e-2
)
ott.assert_almost_equal(
    algo_2d.getShift().norm(), bonferroni_index / math.sqrt(2.0), 1e-2, 1e-2
)

# ------------------------------------------------------- outer convergence
algo_iter = otrobopt.SORARobustOptimizationAlgorithm(problem, ot.Cobyla())
algo_iter.setStartingPoint([0.0])
algo_iter.setMaximumIterationNumber(0)
algo_iter.run()
assert algo_iter.getResult().getStatus() == ot.OptimizationResult.FAILURE
assert "EXACT" in algo_iter.getResult().getStatusMessage()

# ---------------------------------------------------------- solver and copy
inner = ot.Cobyla()
inner.setMaximumIterationNumber(500)
algo.setInnerOptimizationAlgorithm(inner)
assert algo.getInnerOptimizationAlgorithm().getMaximumIterationNumber() == 500
outer = ot.Cobyla()
outer.setMaximumIterationNumber(300)
algo.setOptimizationAlgorithm(outer)
assert algo.getOptimizationAlgorithm().getMaximumIterationNumber() == 300
copy = otrobopt.SORARobustOptimizationAlgorithm(algo)
assert (
    copy.getJointChanceTreatment()
    == otrobopt.SORARobustOptimizationAlgorithm.EXACT
)
assert copy.getOptimizationAlgorithm().getMaximumIterationNumber() == 300

# ------------------------------------------------------- serialization
handle, fileName = tempfile.mkstemp(suffix=".xml")
os.close(handle)
try:
    study = ot.Study()
    study.setStorageManager(ot.XMLStorageManager(fileName))
    study.add("algo", algo)
    study.save()
    study = ot.Study()
    study.setStorageManager(ot.XMLStorageManager(fileName))
    study.load()
    reloaded = otrobopt.SORARobustOptimizationAlgorithm()
    study.fillObject("algo", reloaded)
    assert (
        reloaded.getJointChanceTreatment()
        == otrobopt.SORARobustOptimizationAlgorithm.EXACT
    )
    assert reloaded.getStartingPoint() == [0.0]
    reloaded.run()
    ott.assert_almost_equal(
        reloaded.getResult().getOptimalPoint(), [expected], 5e-3, 5e-3
    )
finally:
    os.remove(fileName)

print("OK")
