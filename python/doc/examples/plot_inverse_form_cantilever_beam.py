"""
Inverse FORM with a cantilever beam
===================================
"""

# %%
# This example uses the cantilever beam model from the OpenTURNS use cases
# to demonstrate the :class:`~otrobopt.InverseFORM` algorithm.
#
# The beam tip displacement is given by:
#
# .. math::
#
#     d = \frac{F L^3}{3 E I}
#
# where :math:`E` is Young's modulus, :math:`F` is the load,
# :math:`L` is the length, and :math:`I` is the moment of inertia.
#
# We treat the length :math:`L` as a parameter to calibrate. Given a
# target reliability index :math:`\beta_t`, we seek the beam length
# such that the Hasofer-Lind reliability index of the failure event
# :math:`\{d > d_0\}` (displacement exceeding a threshold) equals
# :math:`\beta_t`.

# %%
import openturns as ot
import otrobopt
from openturns.usecases import cantilever_beam

ot.RandomGenerator.SetSeed(0)

# Load the cantilever beam use case
cb = cantilever_beam.CantileverBeam()
print(cb.model)

# %%
# Freeze the beam length L (input index 2) as a parameter with initial value 2.55 m.
# The remaining random inputs are E, F, I.
g = ot.ParametricFunction(cb.model, [2], [2.55])

# Build the joint distribution of the random inputs (E, F, I)
marginals = [cb.E, cb.F, cb.II]
distribution = ot.JointDistribution(marginals)

# Starting point: median values of the random inputs
x0 = [dist.computeQuantile(0.5)[0] for dist in marginals]

# %%
# Define the failure event: displacement > 0.15 m.
vect = ot.RandomVector(distribution)
output = ot.CompositeRandomVector(g, vect)
event = ot.ThresholdEvent(output, ot.Greater(), 0.15)

# %%
# Run the inverse FORM algorithm with a target reliability index
# :math:`\beta_t = 3.0`.
algo = otrobopt.InverseFORM(event, 'L', x0)
algo.setTargetBeta(3.0)
algo.run()

result = algo.getResult()
print('Calibrated L =', result.getParameter())
print('Hasofer-Lind beta =', result.getHasoferReliabilityIndex())
print('Convergence criteria =', result.getConvergenceCriteria())

# %%
# The calibrated length :math:`L^*` is the beam length that achieves
# the target reliability index :math:`\beta_t = 3.0` for the given
# failure threshold. If the initial length leads to a reliability
# above the target, the algorithm increases it (making the beam more
# flexible and thus less reliable), and vice versa.
