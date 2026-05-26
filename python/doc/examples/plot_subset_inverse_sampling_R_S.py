"""
Subset inverse sampling on an R-S model
=======================================
"""

# %%
# This example demonstrates :class:`~otrobopt.SubsetInverseSampling` on a
# simple Resistance-Stress (R-S) model.
#
# The limit state function is:
#
# .. math::
#
#     G(\mathbf{U}) = U_1 - U_2
#
# where :math:`U_1 \sim \mathcal{N}(7, 1)` represents the resistance and
# :math:`U_2 \sim \mathcal{N}(2, 1)` represents the stress. Failure occurs
# when :math:`G < 0`.
#
# The subset inverse simulation algorithm estimates the threshold value
# :math:`q` such that:
#
# .. math::
#
#     \mathbb{P}(G(\mathbf{U}) \leq q) = p_t
#
# where :math:`p_t` is a target probability. This is the inverse problem:
# instead of computing the probability for a given threshold, we find the
# threshold for a given probability.

# %%
import openturns as ot
import otrobopt

ot.RandomGenerator.SetSeed(0)

# %%
# Define the limit state function and the probabilistic model.
limitState = ot.SymbolicFunction(['u1', 'u2'], ['u1 - u2'])
dim = limitState.getInputDimension()

mean = [7.0, 2.0]
sigma = [1.0, 1.0]
R = ot.IdentityMatrix(dim)
distribution = ot.Normal(mean, sigma, R)

# Build the event
vect = ot.RandomVector(distribution)
output = ot.CompositeRandomVector(limitState, vect)
event = ot.ThresholdEvent(output, ot.Less(), 0.0)

# %%
# Run the subset inverse sampling algorithm for a target probability
# :math:`p_t = 2 \times 10^{-4}`.
targetProbability = 0.0002

algo = otrobopt.SubsetInverseSampling(event, targetProbability)
algo.setMaximumOuterSampling(10000)
algo.run()

# %%
# Retrieve and display the results.
result = algo.getResult()
pf = result.getProbabilityEstimate()
threshold = algo.getThresholdPerStep()[-1]
threshold_cl = algo.getThresholdConfidenceLength(0.90)

print('Target probability =', targetProbability)
print('Estimated probability =', pf)
print('Threshold at target =', threshold)
print('Threshold confidence length (90%) =', threshold_cl)
print('Number of steps =', algo.getNumberOfSteps())
print('Coefficient of variation =', result.getCoefficientOfVariation())
