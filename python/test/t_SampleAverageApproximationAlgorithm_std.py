#!/usr/bin/env python

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


# --------------------------------------------------------- empty algorithm
empty = otrobopt.SampleAverageApproximationAlgorithm()
assert "SampleAverageApproximationAlgorithm" in repr(empty)
assert empty.getSampleSize() > 0
assert empty.getValidationSampleSize() > 0
with ott.assert_raises(Exception):
    empty.run()
with ott.assert_raises(Exception):
    empty.getSafeProbability()
with ott.assert_raises(Exception):
    empty.getValidationSafeProbability()
with ott.assert_raises(Exception):
    empty.getValidationConfidenceInterval()
with ott.assert_raises(Exception):
    empty.setSampleSize(0)
with ott.assert_raises(Exception):
    empty.setValidationSampleSize(0)

plain = ot.OptimizationProblem()
plain.setObjective(ot.SymbolicFunction(["x"], ["x^2"]))
with ott.assert_raises(Exception):
    empty.setProblem(plain)

# ------------------------------------------------------- 1D Greater target
ot.RandomGenerator.SetSeed(12345)
problem = make_problem()
algo = otrobopt.SampleAverageApproximationAlgorithm(
    problem, ot.Cobyla(), 2000
)
algo.setValidationSampleSize(5000)
algo.setStartingPoint([0.0])
algo.run()
expected = ot.Normal().computeQuantile(0.1)[0]
ott.assert_almost_equal(
    algo.getResult().getOptimalPoint(), [expected], 0.0, 1e-1
)
assert algo.getSample().getSize() == 2000
assert algo.getSample().getDimension() == 1
assert algo.getValidationSample().getSize() == 5000
assert algo.getSafeProbability() >= 0.9
assert algo.getSafeProbability() <= 0.92
assert abs(algo.getValidationSafeProbability() - 0.9) < 0.05
interval = algo.getValidationConfidenceInterval()
assert interval.getDimension() == 1
assert interval.getLowerBound()[0] <= algo.getValidationSafeProbability()
assert algo.getValidationSafeProbability() <= interval.getUpperBound()[0]
assert interval.getLowerBound()[0] >= 0.0
assert interval.getUpperBound()[0] <= 1.0
assert "SampleAverageApproximationAlgorithm" in repr(algo)

# ---------------------------------------------------------- 1D Less target
ot.RandomGenerator.SetSeed(54321)
problem_less = make_problem(ot.Less(), 0.1)
algo_less = otrobopt.SampleAverageApproximationAlgorithm(
    problem_less, ot.Cobyla(), 2000
)
algo_less.setValidationSampleSize(5000)
algo_less.setStartingPoint([0.0])
algo_less.run()
expected_less = ot.Normal().computeQuantile(0.9)[0]
ott.assert_almost_equal(
    algo_less.getResult().getOptimalPoint(), [expected_less], 0.0, 1e-1
)
assert algo_less.getSafeProbability() >= 0.9
assert algo_less.getSafeProbability() <= 0.92
assert abs(algo_less.getValidationSafeProbability() - 0.9) < 0.05

# ------------------------------------------------------- explicit samples
calibration = ot.Normal().getSample(1000)
validation = ot.Normal().getSample(2000)
first = otrobopt.SampleAverageApproximationAlgorithm(
    problem, ot.Cobyla(), 2000
)
first.setSample(calibration)
first.setValidationSample(validation)
first.setStartingPoint([0.0])
first.run()
assert first.getSample().getSize() == 1000
assert first.getValidationSample().getSize() == 2000
second = otrobopt.SampleAverageApproximationAlgorithm(
    problem, ot.Cobyla(), 2000
)
second.setSample(calibration)
second.setValidationSample(validation)
second.setStartingPoint([0.0])
second.run()
ott.assert_almost_equal(
    first.getResult().getOptimalPoint(),
    second.getResult().getOptimalPoint(),
)
ott.assert_almost_equal(
    first.getSafeProbability(), second.getSafeProbability()
)
ott.assert_almost_equal(
    first.getValidationSafeProbability(),
    second.getValidationSafeProbability(),
)

# A sample with the wrong dimension is rejected.
bad = otrobopt.SampleAverageApproximationAlgorithm(problem, ot.Cobyla(), 100)
bad.setSample(ot.Normal(2).getSample(10))
with ott.assert_raises(Exception):
    bad.run()

# ------------------------------------------------ solver and sample sizes
solver = ot.Cobyla()
solver.setMaximumIterationNumber(400)
algo.setOptimizationAlgorithm(solver)
assert algo.getOptimizationAlgorithm().getMaximumIterationNumber() == 400
algo.setSampleSize(1500)
algo.setValidationSampleSize(2500)
assert algo.getSampleSize() == 1500
assert algo.getValidationSampleSize() == 2500
algo.setSample(ot.Sample())
algo.setValidationSample(ot.Sample())
algo.run()
assert algo.getSample().getSize() == 1500
assert algo.getValidationSample().getSize() == 2500

print("OK")
