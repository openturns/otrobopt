#!/usr/bin/env python

import openturns as ot
import openturns.testing as ott
import otrobopt


def make_problem(operator=ot.GreaterOrEqual(), alpha=0.9, size=1):
    if size == 1:
        limit_state = ot.ParametricFunction(
            ot.SymbolicFunction(["x", "theta"], ["theta - x"]), [1], [0.0]
        )
        distribution = ot.Normal()
    else:
        limit_state = ot.ParametricFunction(
            ot.SymbolicFunction(
                ["x", "t1", "t2"], ["t1 - x", "t2 - x"]
            ),
            [1, 2],
            [0.0, 0.0],
        )
        distribution = ot.JointDistribution([ot.Normal(), ot.Normal()])
    measure = otrobopt.JointChanceMeasure(
        limit_state, distribution, operator, alpha
    )
    return otrobopt.RBDOProblem(
        ot.SymbolicFunction(["x"], ["x^2"]), measure
    )


# --------------------------------------------------------- empty algorithm
empty = otrobopt.DoubleLoopRBDOAlgorithm()
assert "DoubleLoopRBDOAlgorithm" in repr(empty)
with ott.assert_raises(Exception):
    empty.run()
with ott.assert_raises(Exception):
    empty.getReliabilityIndex()
with ott.assert_raises(Exception):
    empty.getPerformanceMeasure()
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
algo = otrobopt.DoubleLoopRBDOAlgorithm(problem, ot.Cobyla())
assert algo.getMethod() == otrobopt.DoubleLoopRBDOAlgorithm.PMA
assert algo.getJointChanceTreatment() == otrobopt.DoubleLoopRBDOAlgorithm.EXACT
assert algo.getRBDOProblem().getTargetProbability() == 0.9
assert "PMA" in repr(algo)
with ott.assert_raises(Exception):
    otrobopt.DoubleLoopRBDOAlgorithm(make_problem(alpha=0.5), ot.Cobyla())
with ott.assert_raises(Exception):
    otrobopt.DoubleLoopRBDOAlgorithm(make_problem(alpha=0.4), ot.Cobyla())
# For a Less operator the safe-event probability is 1 - alpha.
with ott.assert_raises(Exception):
    otrobopt.DoubleLoopRBDOAlgorithm(
        make_problem(ot.Less(), 0.6), ot.Cobyla()
    )
otrobopt.DoubleLoopRBDOAlgorithm(make_problem(ot.Less(), 0.4), ot.Cobyla())

# ------------------------------------------------- 1D Greater than target
expected = ot.Normal().computeQuantile(0.1)[0]
target_index = ot.DistFunc.qNormal(0.9)
for method in [
    otrobopt.DoubleLoopRBDOAlgorithm.PMA,
    otrobopt.DoubleLoopRBDOAlgorithm.RIA,
]:
    algo = otrobopt.DoubleLoopRBDOAlgorithm(problem, ot.Cobyla())
    algo.setMethod(method)
    algo.setStartingPoint([0.0])
    algo.run()
    ott.assert_almost_equal(
        algo.getResult().getOptimalPoint(), [expected], 5e-3, 5e-3
    )
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
    assert "RIA" in repr(algo) or "PMA" in repr(algo)

# ----------------------------------------------------- 1D Less than target
problem_less = make_problem(ot.Less(), 0.1)
expected_less = ot.Normal().computeQuantile(0.9)[0]
for method in [
    otrobopt.DoubleLoopRBDOAlgorithm.PMA,
    otrobopt.DoubleLoopRBDOAlgorithm.RIA,
]:
    algo = otrobopt.DoubleLoopRBDOAlgorithm(problem_less, ot.Cobyla())
    algo.setMethod(method)
    algo.setStartingPoint([0.0])
    algo.run()
    ott.assert_almost_equal(
        algo.getResult().getOptimalPoint(), [expected_less], 5e-3, 5e-3
    )
    ott.assert_almost_equal(
        algo.getReliabilityIndex(), target_index, 1e-2, 1e-2
    )

# ------------------------------------------------ vector joint chance modes
problem_vector = make_problem(size=2)
algo = otrobopt.DoubleLoopRBDOAlgorithm(problem_vector, ot.Cobyla())
algo.setMethod(otrobopt.DoubleLoopRBDOAlgorithm.PMA)
algo.setStartingPoint([0.0])

# Exact joint event: one scalarized limit state with the target index.
algo.setJointChanceTreatment(otrobopt.DoubleLoopRBDOAlgorithm.EXACT)
assert algo.getJointChanceTreatment() == otrobopt.DoubleLoopRBDOAlgorithm.EXACT
assert "EXACT" in repr(algo)
algo.run()
ott.assert_almost_equal(
    algo.getResult().getOptimalPoint(), [expected], 5e-3, 5e-3
)
ott.assert_almost_equal(algo.getTargetReliabilityIndex(), target_index)

# Bonferroni: the component target is 1 - (1 - alpha) / n.
algo.setJointChanceTreatment(otrobopt.DoubleLoopRBDOAlgorithm.BONFERRONI)
assert "BONFERRONI" in repr(algo)
bonferroni_index = ot.DistFunc.qNormal(0.95)
ott.assert_almost_equal(algo.getTargetReliabilityIndex(), bonferroni_index)
algo.run()
ott.assert_almost_equal(
    algo.getResult().getOptimalPoint(),
    [-bonferroni_index],
    5e-3,
    5e-3,
)
ott.assert_almost_equal(
    algo.getReliabilityIndex(), bonferroni_index, 1e-2, 1e-2
)

# ---------------------------------------------------------- solver and copy
inner = ot.Cobyla()
inner.setMaximumIterationNumber(500)
algo.setInnerOptimizationAlgorithm(inner)
assert algo.getInnerOptimizationAlgorithm().getMaximumIterationNumber() == 500
outer = ot.Cobyla()
outer.setMaximumIterationNumber(300)
algo.setOptimizationAlgorithm(outer)
assert algo.getOptimizationAlgorithm().getMaximumIterationNumber() == 300
copy = otrobopt.DoubleLoopRBDOAlgorithm(algo)
assert copy.getMethod() == otrobopt.DoubleLoopRBDOAlgorithm.PMA
assert (
    copy.getJointChanceTreatment()
    == otrobopt.DoubleLoopRBDOAlgorithm.BONFERRONI
)

print("OK")
