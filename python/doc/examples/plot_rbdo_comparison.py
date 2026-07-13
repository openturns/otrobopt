"""
Comparison of RIA, PMA and SORA on a simple RBDO problem
=========================================================
"""

# %%
# This example compares the three RBDO algorithms on the simple
# analytical problem from the `Comparaison_otrobopt.ipynb` notebook
# in `l-brevault/RBDO_OT <https://github.com/l-brevault/RBDO_OT>`_.
#
# .. math::
#
#     \begin{aligned}
#     & \underset{d}{\text{minimize}}
#     & & (d_0 - 0.25)^2 - d_1 - d_0 \\
#     & \text{subject to}
#     & & \mathbb{P}\big(Z - d_1 - 2 d_0 \leq 0\big) \leq 0.02275 \\
#     & & & d \in [-5, 5]^2
#     \end{aligned}
#
# where :math:`Z \sim \mathcal{N}(0, 1)` is a standard Normal random
# variable.

# %%
import openturns as ot
import otrobopt

ot.RandomGenerator.SetSeed(0)

# %%
objective = ot.SymbolicFunction(['d0', 'd1'], ['(d0 - 0.25)^2 - d1 - d0'])

# %%
# The constraint is :math:`g(d, Z) = -(Z - d_1 - 2 d_0) \leq 0`.
# We formulate the limit state function in terms of :math:`d` and
# the random variable :math:`Z`.  Because :class:`~otrobopt.RBDOProblem`
# expects a single random vector, we treat :math:`Z` as an :math:`X`
# variable with a fixed distribution.
g = ot.SymbolicFunction(['d0', 'd1', 'z'], ['-((z - d1) - 2.0 * d0)'])

distX = ot.Normal(0.0, 1.0)
distZ = ot.Distribution()
targetBeta = ot.Normal().computeQuantile(0.97725)[0]  # ~2.0

problem = otrobopt.RBDOProblem(
    objective, g, distX, distZ, targetBeta, ot.Less(), [0, 1], [])
problem.setBounds(ot.Interval([-5.0, -5.0], [5.0, 5.0]))
problem.setDesignDimension(2)
problem.setParameterDimension(0)

solver = ot.Cobyla()
solver.setMaximumIterationNumber(200)

# %%
print('RIA:')
ria = otrobopt.RIAAlgorithm(problem, solver)
ria.setStartingPoint([0.0, 0.0])
ria.run()
xStarRIA = ria.getResult().getOptimalPoint()
print('  d* =', xStarRIA)
print('  f* =', objective(xStarRIA)[0])

# %%
print('PMA:')
pma = otrobopt.PMAAlgorithm(problem, solver)
pma.setStartingPoint([0.0, 0.0])
pma.run()
xStarPMA = pma.getResult().getOptimalPoint()
print('  d* =', xStarPMA)
print('  f* =', objective(xStarPMA)[0])

# %%
print('SORA:')
sora = otrobopt.SORAAlgorithm(problem, solver)
sora.setStartingPoint([0.0, 0.0])
sora.setMaximumSORAIteration(20)
sora.setSORATolerance(1e-3)
sora.run()
xStarSORA = sora.getResult().getOptimalPoint()
print('  d* =', xStarSORA)
print('  f* =', objective(xStarSORA)[0])

# %%
# All three algorithms converge to the same optimum
# :math:`d^* \approx (-0.25,\,-1.5)` with :math:`f^* = 2`.
# The constraint is active at the optimum: the failure probability
# equals the target :math:`\Phi(-2)`.
