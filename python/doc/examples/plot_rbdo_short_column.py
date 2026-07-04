"""
Cantilever beam RBDO with RIA, PMA and SORA
============================================
"""

# %%
# This example applies the three RBDO algorithms on a cantilever beam
# problem inspired by the short column from
# `l-brevault/RBDO_OT <https://github.com/l-brevault/RBDO_OT>`_.
#
# A beam with rectangular cross-section :math:`d_b \times d_h` is
# subjected to a random bending moment :math:`M`.  The objective is
# to minimise the area while satisfying a reliability constraint on
# the bending stress.
#
# .. math::
#
#     \begin{aligned}
#     & \underset{d}{\text{minimize}}
#     & & d_b \, d_h \\
#     & \text{subject to}
#     & & \mathbb{P}\big(g(d, M) \leq 0\big) \leq \Phi(-\beta_t) \\
#     & & & d \in [1,\,100]^2
#     \end{aligned}
#
# with :math:`\beta_t = 3` and
#
# .. math::
#
#     g(d, M) = d_b d_h^2 - M
#
# The bending moment :math:`M \sim \mathcal{N}(1000,\,200)` is the only
# random variable.

# %%
import openturns as ot
import otrobopt

ot.RandomGenerator.SetSeed(0)

# %%
objective = ot.SymbolicFunction(['db', 'dh'], ['db * dh'])

# %%
# Limit state: bending stress must not exceed allowable.
# The bending moment M is the only random variable.
g = ot.SymbolicFunction(
    ['db', 'dh', 'M'],
    ['db * dh^2 - M'])

# %%
distX = ot.Normal(1000.0, 200.0)
distZ = ot.Distribution()
targetBeta = 3.0

problem = otrobopt.RBDOProblem(
    objective, g, distX, distZ, targetBeta, ot.Less(), [0, 1], [])
problem.setBounds(ot.Interval([1.0, 1.0], [100.0, 100.0]))
problem.setDesignDimension(2)
problem.setParameterDimension(0)

solver = ot.Cobyla()
solver.setMaximumIterationNumber(1000)

# %%
print('RIA:')
ria = otrobopt.RIAAlgorithm(problem, solver)
ria.setStartingPoint([10.0, 10.0])
ria.run()
xStarRIA = ria.getResult().getOptimalPoint()
print('  d* =', xStarRIA)
print('  area =', objective(xStarRIA)[0])

# %%
print('PMA:')
pma = otrobopt.PMAAlgorithm(problem, solver)
pma.setStartingPoint([10.0, 10.0])
pma.run()
xStarPMA = pma.getResult().getOptimalPoint()
print('  d* =', xStarPMA)
print('  area =', objective(xStarPMA)[0])

# %%
print('SORA:')
sora = otrobopt.SORAAlgorithm(problem, solver)
sora.setStartingPoint([10.0, 10.0])
sora.setMaximumSORAIteration(20)
sora.setSORATolerance(1e-3)
sora.run()
xStarSORA = sora.getResult().getOptimalPoint()
print('  d* =', xStarSORA)
print('  area =', objective(xStarSORA)[0])
