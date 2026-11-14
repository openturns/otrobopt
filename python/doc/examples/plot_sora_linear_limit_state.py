r"""
SORA on a linear limit state
============================

This example applies :class:`~otrobopt.SORARobustOptimizationAlgorithm` to a
one-dimensional reliability-based design problem with a linear limit state.

Problem statement
-----------------

.. math::

    \begin{aligned}
    & \underset{x}{\text{minimize}}
    & & x^2 \\
    & \text{subject to}
    & & \mathbb{P}_{\Theta}(\Theta - x \ge 0) \ge 0.9, \\
    & & & \Theta \sim \mathcal{N}(0, 1).
    \end{aligned}

The safe event is :math:`\{\Theta \ge x\}`.  Its probability is
:math:`1-\Phi(x)`, so the analytic solution is
:math:`x^* = -\Phi^{-1}(0.9) \approx -1.2816`.

SORA alternates a deterministic design optimization with a
performance-measure analysis.  The physical shift is
:math:`d = x - \theta^*`, where :math:`\theta^*` is the physical image of the
most probable point.  This example reports the reliability index, the
approximate probability, the function evaluation counts and the error with
respect to the analytic solution.
"""

# %%
import openturns as ot
import openturns.viewer as otv
import otrobopt

# %%
# Define the objective and the limit state.  Both are wrapped in
# :class:`~openturns.MemoizeFunction` objects with the cache disabled, so that
# the final counts report the actual number of evaluations.
objective = ot.MemoizeFunction(ot.SymbolicFunction(["x"], ["x^2"]))
objective.disableCache()
limitState = ot.MemoizeFunction(
    ot.ParametricFunction(
        ot.SymbolicFunction(["x", "theta"], ["theta - x"]), [1], [0.0]
    )
)
limitState.disableCache()

measure = otrobopt.JointChanceMeasure(
    limitState, ot.Normal(), ot.GreaterOrEqual(), 0.9
)
problem = otrobopt.RBDOProblem(objective, measure)

# %%
# Solve with SORA.  The outer and inner solvers default to Cobyla, which does
# not require the limit-state gradient.
solver = ot.Cobyla()
solver.setMaximumIterationNumber(500)
algo = otrobopt.SORARobustOptimizationAlgorithm(problem, solver)
algo.setStartingPoint([0.0])
algo.run()

result = algo.getResult()
design = result.getOptimalPoint()[0]
print("Optimal design           :", design)
print("Target reliability index :", algo.getTargetReliabilityIndex())
print("Reliability index        :", algo.getReliabilityIndex())
print("Approximate probability  :", algo.getApproximateProbability())
print("Physical shift           :", algo.getShift())
print("Objective evaluations    :", objective.getInputHistory().getSize())
print("Limit-state evaluations  :", limitState.getInputHistory().getSize())
print("Status                   :", result.getStatusMessage())

# %%
# Ground truth.  For this linear Gaussian problem the optimal design and the
# safe probability are known analytically.
normal = ot.Normal()
analyticDesign = normal.computeQuantile(0.1)[0]
trueProbability = 1.0 - normal.computeCDF(design)
print()
print("Ground truth design      :", analyticDesign)
print("Design error             :", abs(design - analyticDesign))
print("True probability         :", trueProbability)
print("Probability error        :", abs(algo.getApproximateProbability() - trueProbability))
print("Objective ground truth   :", analyticDesign**2)
print("Objective error          :", abs(result.getOptimalValue()[0] - analyticDesign**2))

# %%
# Plot the objective and the optimal design.  The vertical line marks the
# SORA solution, which coincides with the target quantile of the distribution.
xMin = -2.5
xMax = 0.5
abscissas = [xMin + i * (xMax - xMin) / 200.0 for i in range(201)]
graph = ot.Graph("SORA on a linear limit state", "x", "f(x)", True, "topright")
objectiveCurve = ot.Curve(ot.Sample([[x, x * x] for x in abscissas]))
objectiveCurve.setColor("blue")
objectiveCurve.setLegend("objective")
graph.add(objectiveCurve)
optimumCurve = ot.Curve(
    ot.Point([design, design]), ot.Point([0.0, design * design])
)
optimumCurve.setColor("red")
optimumCurve.setLegend("SORA optimum")
graph.add(optimumCurve)
graph.setLegendPosition("topright")
view = otv.View(graph)
