"""
RBDO with three methods on an analytical limit state
=====================================================
"""

# %%
# This example compares the three RBDO algorithms -- RIA, PMA, and SORA --
# available in otrobopt on a simple analytical problem inspired by
# `l-brevault/RBDO_OT <https://github.com/l-brevault/RBDO_OT>`_.
#
# The design variables :math:`d = (d_0, d_1)` appear as deterministic
# parameters of the limit state, while the random vector
# :math:`X = (X_1, X_2)` follows a fixed Normal distribution.
#
# .. math::
#
#     \begin{aligned}
#     & \underset{d}{\text{minimize}}
#     & & d_0 + d_1 \\
#     & \text{subject to}
#     & & \mathbb{P}\big(g(d, X) \leq 0\big) \leq \Phi(-\beta_t) \\
#     & & & d \in [0, 10]^2
#     \end{aligned}
#
# with :math:`\beta_t = 2` (target reliability index),
# :math:`X \sim \mathcal{N}((0,0),\, 0.6^2 I_2)` and
#
# .. math::
#
#     g(d, X) = \frac{(d_0 + X_1)^2\,(d_1 + X_2)}{20} - 1
#

# %%
import openturns as ot
import otrobopt

ot.RandomGenerator.SetSeed(0)

# %%
objective = ot.SymbolicFunction(['d0', 'd1'], ['d0 + d1'])

g = ot.SymbolicFunction(
    ['d0', 'd1', 'x1', 'x2'],
    ['(d0 + x1)^2 * (d1 + x2) / 20.0 - 1.0'])

distX = ot.Normal([0.0, 0.0], [0.6, 0.6], ot.IdentityMatrix(2))
distZ = ot.Distribution()
targetBeta = 2.0

problem = otrobopt.RBDOProblem(
    objective, g, distX, distZ, targetBeta, ot.Less(), [0, 1], [])
problem.setBounds(ot.Interval([0.0, 0.0], [10.0, 10.0]))
problem.setDesignDimension(2)
problem.setParameterDimension(0)

solver = ot.Cobyla()
solver.setMaximumIterationNumber(200)

# %%
# **RIA** solves the outer problem :math:`\min_d f(d)` subject to
# :math:`\beta(d) - \beta_t \geq 0`, where :math:`\beta(d)` is the
# Hasofer-Lind index obtained by FORM at each candidate design.
print('RIA:')
ria = otrobopt.RIAAlgorithm(problem, solver)
ria.setStartingPoint([5.0, 5.0])
ria.run()
xStarRIA = ria.getResult().getOptimalPoint()
print('  d* =', xStarRIA)
print('  f* =', objective(xStarRIA)[0])

# %%
# **PMA** replaces the reliability constraint by the performance measure
# :math:`g(d, x_{\text{MPTP}}) \geq 0`, where the Most Probable Target
# Point is found by inverse FORM on the sphere :math:`\|u\| = \beta_t`.
print('PMA:')
pma = otrobopt.PMAAlgorithm(problem, solver)
pma.setStartingPoint([5.0, 5.0])
pma.run()
xStarPMA = pma.getResult().getOptimalPoint()
print('  d* =', xStarPMA)
print('  f* =', objective(xStarPMA)[0])

# %%
# **SORA** decouples the deterministic optimization and the reliability
# analysis.  It alternates between solving a shifted deterministic problem
# and updating the shifting vector by inverse FORM.
print('SORA:')
sora = otrobopt.SORAAlgorithm(problem, solver)
sora.setStartingPoint([5.0, 5.0])
sora.setMaximumSORAIteration(20)
sora.setSORATolerance(1e-3)
sora.run()
xStarSORA = sora.getResult().getOptimalPoint()
print('  d* =', xStarSORA)
print('  f* =', objective(xStarSORA)[0])
