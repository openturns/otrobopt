#!/usr/bin/env python
"""Test that RBDOProblem can be solved via Cobyla (a classical OptimizationAlgorithm)."""

import openturns as ot
import otrobopt

# Analytical RBDO test case
# Limit state: g(d1, d2, x1, x2) = d1 + d2 - x1 - x2
# Objective: f(d) = d0 + d1
# X ~ N(0, 1), independent
# beta_target = 3.0

# Objective
objective = ot.SymbolicFunction(["d0", "d1"], ["d0 + d1"])

# Limit state
g = ot.SymbolicFunction(["d1", "d2", "x1", "x2"], ["d1 + d2 - x1 - x2"])

# Distribution of X
distX = ot.Normal(2)

# No Z
distZ = ot.Distribution()

# Target beta
targetBeta = 3.0

# Active indices
activeD = [0, 1]
activeP = []

# Bounds
bounds = ot.Interval([0.0] * 2, [10.0] * 2)

# Build RBDO problem
problem = otrobopt.RBDOProblem(
    objective, g, distX, distZ, targetBeta, ot.Less(), activeD, activeP
)
problem.setBounds(bounds)
problem.setDesignDimension(2)
problem.setParameterDimension(0)

# Cobyla solver
solver = ot.Cobyla()
solver.setMaximumIterationNumber(200)
solver.setMaximumCallsNumber(2000)

# ---- RIA with Cobyla ----
ria = otrobopt.RIAAlgorithm(problem, solver)
ria.setStartingPoint([5.0] * 2)
ria.run()
xStarRIA = ria.getResult().getOptimalPoint()
print("RIA x*=", xStarRIA, " sum=", xStarRIA[0] + xStarRIA[1])
assert (
    abs(xStarRIA[0] + xStarRIA[1] - 4.24264) < 0.5
), f"RIA sum too far from 4.24264: {xStarRIA[0] + xStarRIA[1]}"

# ---- PMA with Cobyla ----
pma = otrobopt.PMAAlgorithm(problem, solver)
pma.setStartingPoint([5.0] * 2)
pma.run()
xStarPMA = pma.getResult().getOptimalPoint()
print("PMA x*=", xStarPMA, " sum=", xStarPMA[0] + xStarPMA[1])
assert (
    abs(xStarPMA[0] + xStarPMA[1] - 4.24264) < 0.5
), f"PMA sum too far from 4.24264: {xStarPMA[0] + xStarPMA[1]}"

# ---- SORA with Cobyla ----
sora = otrobopt.SORAAlgorithm(problem, solver)
sora.setStartingPoint([5.0] * 2)
sora.setMaximumSORAIteration(20)
sora.setSORATolerance(1e-3)
sora.run()
xStarSORA = sora.getResult().getOptimalPoint()
print("SORA x*=", xStarSORA, " sum=", xStarSORA[0] + xStarSORA[1])
assert (
    abs(xStarSORA[0] + xStarSORA[1] - 4.24264) < 0.5
), f"SORA sum too far from 4.24264: {xStarSORA[0] + xStarSORA[1]}"

print("All tests passed!")
