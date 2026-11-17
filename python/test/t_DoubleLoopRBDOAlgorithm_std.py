#!/usr/bin/env python

import os
import tempfile

import openturns as ot
import openturns.testing as ott
import otrobopt


def make_problem(operator=ot.GreaterOrEqual(), alpha=0.9, size=1, distribution=None):
    if size == 1:
        limit_state = ot.ParametricFunction(
            ot.SymbolicFunction(["x", "theta"], ["theta - x"]), [1], [0.0]
        )
        if distribution is None:
            distribution = ot.Normal()
    else:
        limit_state = ot.ParametricFunction(
            ot.SymbolicFunction(
                ["x", "t1", "t2"], ["t1 - x", "t2 - x"]
            ),
            [1, 2],
            [0.0, 0.0],
        )
        if distribution is None:
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

# -------------------------------- 1D Student generalized reliability index
# The standard space of a Student distribution is the same-kind spherical
# space, so the target index is the Student quantile and the FORM probability
# is the standard marginal CDF at the index (Lebrun and Dutfoy, 2009).  The
# linear 1D problem is solved exactly by both the RIA and PMA formulations.
student_distribution = ot.Student(8.0, 1.0, 2.0)
student_standard = student_distribution.getStandardDistribution().getMarginal(0)
student_index = student_standard.computeScalarQuantile(0.1, True)
student_design = student_distribution.computeQuantile(0.1)[0]
student_problem = make_problem(alpha=0.9, distribution=student_distribution)
for method in [
    otrobopt.DoubleLoopRBDOAlgorithm.PMA,
    otrobopt.DoubleLoopRBDOAlgorithm.RIA,
]:
    algo = otrobopt.DoubleLoopRBDOAlgorithm(student_problem, ot.Cobyla())
    algo.setMethod(method)
    algo.setStartingPoint([0.0])
    algo.run()
    ott.assert_almost_equal(
        algo.getTargetReliabilityIndex(), student_index, 1e-12, 1e-12
    )
    ott.assert_almost_equal(
        algo.getReliabilityIndex(), student_index, 5e-3, 5e-3
    )
    ott.assert_almost_equal(
        algo.getApproximateProbability(), 0.9, 5e-3, 5e-3
    )
    ott.assert_almost_equal(
        algo.getResult().getOptimalPoint(), [student_design], 5e-3, 5e-3
    )
    ott.assert_almost_equal(
        algo.getStandardSpaceDesignPoint(), [-student_index], 5e-3, 5e-3
    )
    ott.assert_almost_equal(
        algo.getPhysicalSpaceDesignPoint(), [student_design], 5e-3, 5e-3
    )

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

# ------------------------------------- PMA requires a safe standard origin
# For G(x;u) = u^2 - x the failure domain contains the origin as soon as
# x > 0: a positive value on the target sphere must not certify safety.  The
# performance measure is capped by the value at the origin, so the algorithm
# must return a safe design instead of the deterministic optimum x = 1.
unsafe_origin_limit_state = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "u"], ["u^2 - x"]), [1], [0.0]
)
unsafe_origin_problem = otrobopt.RBDOProblem(
    ot.SymbolicFunction(["x"], ["(x - 1)^2"]),
    otrobopt.JointChanceMeasure(
        unsafe_origin_limit_state, ot.Normal(), ot.GreaterOrEqual(), 0.9
    ),
)
pma_algo = otrobopt.DoubleLoopRBDOAlgorithm(unsafe_origin_problem, ot.Cobyla())
pma_algo.setMethod(otrobopt.DoubleLoopRBDOAlgorithm.PMA)
pma_algo.setStartingPoint([0.5])
pma_algo.run()
design = pma_algo.getResult().getOptimalPoint()[0]
assert design <= 1e-3
# Exact safe probability: 2 - 2 * Phi(sqrt(x)) for x > 0, 1 otherwise.
assert 2.0 - 2.0 * ot.Normal().computeCDF(max(design, 0.0) ** 0.5) >= 0.9

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

# ------------------------------------------- gradient-based inner solver
# The specialized gradients must follow the OpenTURNS orientation, that is the
# input dimension by the output dimension.  AbdoRackwitz assumes a column
# gradient and fails on a transposed one as soon as the standard space is
# multi-dimensional.
smooth_limit_state = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "t1", "t2"], ["t1 + t2 - x"]), [1, 2], [0.0, 0.0]
)
smooth_problem = otrobopt.RBDOProblem(
    ot.SymbolicFunction(["x"], ["x^2"]),
    otrobopt.JointChanceMeasure(
        smooth_limit_state,
        ot.JointDistribution([ot.Normal(), ot.Normal()]),
        ot.GreaterOrEqual(),
        0.9,
    ),
)
gradient_algo = otrobopt.DoubleLoopRBDOAlgorithm(
    smooth_problem, ot.Cobyla(), ot.AbdoRackwitz()
)
gradient_algo.setMethod(otrobopt.DoubleLoopRBDOAlgorithm.RIA)
gradient_algo.setStartingPoint([0.0])
gradient_algo.run()
# The sum of two independent standard normals has standard deviation sqrt(2).
ott.assert_almost_equal(
    gradient_algo.getResult().getOptimalPoint(),
    [2.0 ** 0.5 * expected],
    5e-3,
    5e-3,
)
ott.assert_almost_equal(
    gradient_algo.getReliabilityIndex(), target_index, 1e-2, 1e-2
)
ott.assert_almost_equal(
    gradient_algo.getStandardSpaceDesignPoint(),
    [-target_index / 2.0 ** 0.5] * 2,
    5e-3,
    5e-3,
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

# The default inner solver budget is a number of calls: Cobyla stops on its
# calls number, not on its iteration number.
budget_key = "DoubleLoopRBDOAlgorithm-DefaultInnerMaximumCallsNumber"
previous_budget = ot.ResourceMap.GetAsUnsignedInteger(budget_key)
ot.ResourceMap.SetAsUnsignedInteger(budget_key, 77)
try:
    budget_algo = otrobopt.DoubleLoopRBDOAlgorithm(problem, ot.Cobyla())
    assert budget_algo.getInnerOptimizationAlgorithm().getMaximumCallsNumber() == 77
finally:
    ot.ResourceMap.SetAsUnsignedInteger(budget_key, previous_budget)

# ------------------------------------------- serialization of a completed run
# The result itself must survive the round trip: hasRun() alone would advertise
# diagnostics and leave getResult() empty.
algo_saved = otrobopt.DoubleLoopRBDOAlgorithm(problem, ot.Cobyla())
algo_saved.setStartingPoint([0.0])
algo_saved.run()
handle, fileName = tempfile.mkstemp(suffix=".xml")
os.close(handle)
try:
    study = ot.Study()
    study.setStorageManager(ot.XMLStorageManager(fileName))
    study.add("algo", algo_saved)
    study.save()
    study = ot.Study()
    study.setStorageManager(ot.XMLStorageManager(fileName))
    study.load()
    reloaded = otrobopt.DoubleLoopRBDOAlgorithm()
    study.fillObject("algo", reloaded)
    assert reloaded.getResult().getStatus() == ot.OptimizationResult.SUCCESS
    ott.assert_almost_equal(
        reloaded.getResult().getOptimalPoint(), [expected], 5e-3, 5e-3
    )
    ott.assert_almost_equal(
        reloaded.getResult().getOptimalValue(),
        algo_saved.getResult().getOptimalValue(),
    )
    ott.assert_almost_equal(
        reloaded.getReliabilityIndex(), algo_saved.getReliabilityIndex()
    )
    # The reloaded result is associated with the RBDO problem, not with the
    # transient canonical problem whose chance constraint cannot persist.
    reloaded_problem = reloaded.getResult().getProblem()
    assert reloaded_problem.getObjective().getInputDimension() == 1
    assert reloaded_problem.getInequalityConstraint().getOutputDimension() == 0
    assert reloaded_problem.getObjective()(
        reloaded.getResult().getOptimalPoint()
    ) == reloaded.getResult().getOptimalValue()
finally:
    os.remove(fileName)

# ------------------------------- stale diagnostics after a failed run
# run() resets the diagnostics first: a failed run must not report the values
# of a previous successful one.  setStartingPoint() does not clear the run
# flag, so the failure below happens after a completed run.
stale = otrobopt.DoubleLoopRBDOAlgorithm(problem, ot.Cobyla())
stale.setStartingPoint([0.0])
stale.run()
assert stale.getResult().getStatus() == ot.OptimizationResult.SUCCESS
stale.setStartingPoint([0.0, 0.0])
with ott.assert_raises(Exception):
    stale.run()
for getter in [
    stale.getReliabilityIndex,
    stale.getPerformanceMeasure,
    stale.getApproximateProbability,
]:
    with ott.assert_raises(Exception):
        getter()

print("OK")
