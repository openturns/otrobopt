r"""
Comparing SORA with the double-loop algorithm
=============================================

This example solves the same one-dimensional problem with
:class:`~otrobopt.SORARobustOptimizationAlgorithm` and with the two inner
formulations of :class:`~otrobopt.DoubleLoopRBDOAlgorithm`.

The problem is

.. math::

    \min_x x^2
    \quad \text{subject to} \quad
    \mathbb{P}_{\Theta}(\Theta - x \ge 0) \ge 0.9,
    \qquad \Theta \sim \mathcal{N}(0, 1).

All three algorithms should return the same optimal design,
:math:`x^*=-\Phi^{-1}(0.9)\approx-1.2816`.  The comparison shows that SORA
reaches the same reliability target as the double-loop baselines with fewer
reliability analyses.
"""

# %%
import openturns as ot
import otrobopt

# %%
limitState = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta"], ["theta - x"]), [1], [0.0]
)
measure = otrobopt.JointChanceMeasure(
    limitState, ot.Normal(), ot.GreaterOrEqual(), 0.9
)
problem = otrobopt.RBDOProblem(
    ot.SymbolicFunction(["x"], ["x^2"]), measure
)

# %%
# SORA.
sora = otrobopt.SORARobustOptimizationAlgorithm(problem, ot.Cobyla())
sora.setStartingPoint([0.0])
sora.run()

# %%
# Double-loop RIA and PMA.
doubleLoop = []
for method in [
    otrobopt.DoubleLoopRBDOAlgorithm.RIA,
    otrobopt.DoubleLoopRBDOAlgorithm.PMA,
]:
    algo = otrobopt.DoubleLoopRBDOAlgorithm(problem, ot.Cobyla())
    algo.setMethod(method)
    algo.setStartingPoint([0.0])
    algo.run()
    doubleLoop.append(algo)

# %%
# Collect the results.  The reliability index is the diagnostic index at the
# optimal design, and the approximate probability is its FORM probability.
rows = [
    (
        "SORA",
        sora.getResult().getOptimalPoint()[0],
        sora.getReliabilityIndex(),
        sora.getApproximateProbability(),
    ),
    (
        "Double-loop RIA",
        doubleLoop[0].getResult().getOptimalPoint()[0],
        doubleLoop[0].getReliabilityIndex(),
        doubleLoop[0].getApproximateProbability(),
    ),
    (
        "Double-loop PMA",
        doubleLoop[1].getResult().getOptimalPoint()[0],
        doubleLoop[1].getReliabilityIndex(),
        doubleLoop[1].getApproximateProbability(),
    ),
]

print("Algorithm        | design   | reliability index | probability")
print("-----------------+----------+-------------------+------------")
for name, design, index, probability in rows:
    print(f"{name:16s} | {design:8.5f} | {index:17.5f} | {probability:.5f}")

print()
print("Analytic design  :", ot.Normal().computeQuantile(0.1)[0])
print("Target index     :", ot.DistFunc.qNormal(0.9))
