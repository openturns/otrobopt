"""
Spur speed reducer RBDO with seven design variables
====================================================
"""

# %%
# This example applies RIA, PMA and SORA to the spur speed reducer
# problem from `l-brevault/RBDO_OT <https://github.com/l-brevault/RBDO_OT>`_.  The goal is to minimise
# the weight of a speed reducer subject to a reliability constraint
# on the bending stress of the first gear tooth.
#
# The seven design variables :math:`d_1`--:math:`d_7` correspond to
# geometric parameters of the reducer (face width, module, number of
# teeth, shaft diameters, etc.).  The random vector
# :math:`X \sim \mathcal{N}(0,\, (5\times10^{-3})^2 I_7)` models
# manufacturing tolerances.
#
# .. math::
#
#     \begin{aligned}
#     & \underset{d}{\text{minimize}}
#     & & 0.7854 d_1 d_2^2 (3.3333 d_3^2 + 14.9334 d_3 - 43.0934)  \\
#     &   & & \qquad -\, 1.508 d_1 (d_6^2 + d_7^2) \\
#     &   & & \qquad +\, 7.477 (d_6^3 + d_7^3) \\
#     &   & & \qquad +\, 0.7854 (d_4 d_6^2 + d_5 d_7^2) \\
#     & \text{subject to}
#     & & \mathbb{P}\big(g(d, X) \geq 0\big) \geq \Phi(\beta_t) \\
#             & & & d \in [2.6,0.7,17,7.3,7.3,2.9,5.0] \times
#             [3.6,0.8,28,8.3,8.3,3.9,5.5]
#     \end{aligned}
#
# with :math:`\beta_t = 3` and
#
# .. math::
#
#     g(d, X) = 27 - (d_1 + X_1) (d_2 + X_2)^2 (d_3 + X_3)
#
# The limit state expresses that the contact stress must stay below the
# allowable value.

# %%
import openturns as ot
import otrobopt

ot.RandomGenerator.SetSeed(0)

# %%
# Objective function.
objective = ot.SymbolicFunction(
    ['d1', 'd2', 'd3', 'd4', 'd5', 'd6', 'd7'],
    [
        '0.7854 * d1 * d2^2 * (3.3333 * d3^2 + 14.9334 * d3 - 43.0934)'
        ' - 1.508 * d1 * (d6^2 + d7^2)'
        ' + 7.477 * (d6^3 + d7^3)'
        ' + 0.7854 * (d4 * d6^2 + d5 * d7^2)'
    ])

# %%
# Limit state function (bending stress of the first gear tooth).
# Inputs: d1..d7, X1..X7.
g = ot.SymbolicFunction(
    ['d1', 'd2', 'd3', 'd4', 'd5', 'd6', 'd7',
     'x1', 'x2', 'x3', 'x4', 'x5', 'x6', 'x7'],
    ['27.0 - (d1 + x1) * (d2 + x2)^2 * (d3 + x3)'])

# %%
# Distribution of the random perturbations.
distX = ot.Normal([0.0] * 7, [5e-3] * 7, ot.IdentityMatrix(7))
distZ = ot.Distribution()
targetBeta = 3.0

activeD = list(range(7))
problem = otrobopt.RBDOProblem(
    objective, g, distX, distZ, targetBeta, ot.Greater(), activeD, [])
problem.setBounds(ot.Interval(
    [2.6, 0.7, 17.0, 7.3, 7.3, 2.9, 5.0],
    [3.6, 0.8, 28.0, 8.3, 8.3, 3.9, 5.5]))
problem.setDesignDimension(7)
problem.setParameterDimension(0)

riaSolver = ot.Cobyla()
riaSolver.setMaximumIterationNumber(500)

# %%
print('RIA:')
ria = otrobopt.RIAAlgorithm(problem, riaSolver)
ria.setStartingPoint([3.5, 0.7, 17.0, 7.3, 7.7, 3.35, 5.3])
ria.run()
xStarRIA = ria.getResult().getOptimalPoint()
print('  d* =', xStarRIA)
print('  f* =', objective(xStarRIA)[0])

# %%
print('PMA:')
pma = otrobopt.PMAAlgorithm(problem, riaSolver)
pma.setStartingPoint([3.5, 0.7, 17.0, 7.3, 7.7, 3.35, 5.3])
pma.run()
xStarPMA = pma.getResult().getOptimalPoint()
print('  d* =', xStarPMA)
print('  f* =', objective(xStarPMA)[0])

# %%
# SORA uses a derivative-free outer solver (Cobyla) to avoid roundoff
# issues in the inner inverse FORM step.
print('SORA:')
soraSolver = ot.Cobyla()
soraSolver.setMaximumIterationNumber(2000)
sora = otrobopt.SORAAlgorithm(problem, soraSolver)
sora.setStartingPoint([3.5, 0.7, 17.0, 7.3, 7.7, 3.35, 5.3])
sora.setMaximumSORAIteration(15)
sora.setSORATolerance(5e-2)
sora.run()
xStarSORA = sora.getResult().getOptimalPoint()
print('  d* =', xStarSORA)
print('  f* =', objective(xStarSORA)[0])
