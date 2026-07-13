#!/usr/bin/env python
"""Test that RBDOProblem works as a RobustOptimizationProblem and OptimizationProblem."""

import openturns as ot
import openturns.testing as ott
import otrobopt

# Analytical RBDO test case
# Limit state: g(d1, d2, X1, X2) = d1 + d2 - X1 - X2
# Objective: f(d) = d0 + d1
# X ~ N(0, 1), independent; beta_target = 3.0
objective = ot.SymbolicFunction(["d0", "d1"], ["d0 + d1"])
g = ot.SymbolicFunction(["d1", "d2", "x1", "x2"], ["d1 + d2 - x1 - x2"])
distX = ot.Normal(2)
distZ = ot.Distribution()
targetBeta = 3.0
activeD = [0, 1]
activeP = []
bounds = ot.Interval([0.0] * 2, [10.0] * 2)

# ---- Problem construction and properties ----
problem = otrobopt.RBDOProblem(
    objective, g, distX, distZ, targetBeta, ot.Less(), activeD, activeP
)
problem.setBounds(bounds)
problem.setDesignDimension(2)
problem.setParameterDimension(0)

assert problem.hasReliabilityMeasure()
assert not problem.hasRobustnessMeasure()
assert problem.getObjective().getInputDimension() == 2
assert problem.getInequalityConstraint().getInputDimension() == 2
assert problem.getBounds().getDimension() == 2
assert problem.isMinimization()
assert problem.getDesignDimension() == 2
assert problem.getParameterDimension() == 0
ott.assert_almost_equal(problem.getTargetBeta(), 3.0, 1e-3, 1e-3)

# ---- Constraint evaluation at known points ----
# The corrected inequality constraint is P[g >= 0] - Phi(beta) >= 0
# For g = d1+d2 - X1 - X2: P[g >= 0] = P[X1+X2 <= d1+d2] = Phi((d1+d2)/sqrt(2))
constraint = problem.getInequalityConstraint()
# At d=[0,0]: beta-computed=0, beta_target=3, constraint=0-3 = -3
ott.assert_almost_equal(constraint([0.0, 0.0]), [-3.0], 1e-3, 1e-3)
# At d=[2.1213, 2.1213]: Phi(3) - Phi(3) approx 0
ott.assert_almost_equal(constraint([2.1213, 2.1213]), [0.0], 1e-3, 1e-3)
# At d=[3, 3]: beta-computed=4.2426, beta_target=3, constraint=4.2426-3 = 1.2426
ott.assert_almost_equal(constraint([3.0, 3.0]), [1.2426], 1e-3, 1e-3)

# ---- SequentialMonteCarloRobustAlgorithm ----
# Uses the reliability measure directly (discretized empirically)
distX_small = ot.Normal([0.0] * 2, [0.2] * 2, ot.IdentityMatrix(2))
problem_smc = otrobopt.RBDOProblem(
    objective, g, distX_small, distZ, 1.0, ot.Less(), activeD, activeP
)
problem_smc.setBounds(bounds)
solver = ot.NLopt("LD_SLSQP")
solver.setMaximumIterationNumber(10)
algo = otrobopt.SequentialMonteCarloRobustAlgorithm(problem_smc, solver)
algo.setMaximumIterationNumber(3)
algo.setMaximumAbsoluteError(1e-2)
algo.setInitialSamplingSize(5)
algo.setInitialSearch(50)
algo.run()
result = algo.getResult()
xStarSMC = result.getOptimalPoint()
print("SMC x*=", xStarSMC, " f*=", result.getOptimalValue())
assert xStarSMC[0] >= 0.0
assert xStarSMC[1] >= 0.0
assert result.getIterationNumber() >= 1

# ---- RIA algorithm ----
ria_solver = ot.NLopt("LD_SLSQP")
ria_solver.setMaximumIterationNumber(100)
ria = otrobopt.RIAAlgorithm(problem, ria_solver)
ria.setStartingPoint([5.0] * 2)
ria.setMaximumIterationNumber(10)
ria.run()
xStarRIA = ria.getResult().getOptimalPoint()
print("RIA x*=", xStarRIA, " sum=", xStarRIA[0] + xStarRIA[1])
ott.assert_almost_equal(xStarRIA, [2.1213, 2.1213], 1e-2, 1e-2)

# ---- PMA algorithm ----
pma_solver = ot.NLopt("LD_SLSQP")
pma_solver.setMaximumIterationNumber(100)
pma = otrobopt.PMAAlgorithm(problem, pma_solver)
pma.setStartingPoint([5.0] * 2)
pma.setMaximumIterationNumber(10)
pma.run()
xStarPMA = pma.getResult().getOptimalPoint()
print("PMA x*=", xStarPMA, " sum=", xStarPMA[0] + xStarPMA[1])
ott.assert_almost_equal(xStarPMA, [2.1213, 2.1213], 1e-2, 1e-2)

# ---- SORA algorithm ----
sora_solver = ot.NLopt("LD_SLSQP")
sora_solver.setMaximumIterationNumber(100)
sora = otrobopt.SORAAlgorithm(problem, sora_solver)
sora.setStartingPoint([5.0] * 2)
sora.setMaximumIterationNumber(10)
sora.run()
xStarSORA = sora.getResult().getOptimalPoint()
print("SORA x*=", xStarSORA, " sum=", xStarSORA[0] + xStarSORA[1])
ott.assert_almost_equal(xStarSORA, [2.1213, 2.1213], 1e-2, 1e-2)

# ---- Cobyla solves RBDOProblem directly ----
# 1D problem: 1D numerical integration is fast enough
objective_1d = ot.SymbolicFunction(["d"], ["d"])
g_1d = ot.SymbolicFunction(["d", "x1"], ["d - x1"])
distX_1d = ot.Normal(1)
problem_1d = otrobopt.RBDOProblem(
    objective_1d, g_1d, distX_1d, distZ, targetBeta, ot.Less(), [0], []
)
problem_1d.setBounds(ot.Interval([0.0], [10.0]))
# P[g >= 0] = P[d - X1 >= 0] = Phi(d), constraint: Phi(d) - Phi(3) >= 0 => d >= 3
cobyla = ot.Cobyla()
cobyla.setMaximumIterationNumber(30)
cobyla.setMaximumCallsNumber(200)
cobyla.setStartingPoint([5.0])
cobyla.setProblem(problem_1d)
cobyla.run()
xStarCobyla = cobyla.getResult().getOptimalPoint()
print("Cobyla x*=", xStarCobyla)
ott.assert_almost_equal(xStarCobyla, [3.0], 1e-1, 1e-1)

# ---- Greater operator constraint evaluation ----
problem_greater = otrobopt.RBDOProblem(
    objective, g, distX, distZ, targetBeta, ot.Greater(), activeD, activeP
)
problem_greater.setBounds(bounds)
constraint_greater = problem_greater.getInequalityConstraint()
# For Greater: probit transform yields beta_computed - beta_target (same as Less)
ott.assert_almost_equal(constraint_greater([0.0, 0.0]), [-3.0], 1e-3, 1e-3)
ott.assert_almost_equal(constraint_greater([3.0, 3.0]), [1.2426], 1e-3, 1e-3)

print("All tests passed!")
