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
most probable point.  This example also reports the reliability index and the
approximate probability at the solution.
"""

# %%
import openturns as ot
import openturns.viewer as otv
import otrobopt

# %%
# Define the limit state, the joint chance constraint and the problem.
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
# Solve with SORA.  The outer and inner solvers default to Cobyla, which does
# not require the limit-state gradient.
solver = ot.Cobyla()
solver.setMaximumIterationNumber(500)
algo = otrobopt.SORARobustOptimizationAlgorithm(problem, solver)
algo.setStartingPoint([0.0])
algo.run()

result = algo.getResult()
design = result.getOptimalPoint()[0]
targetIndex = algo.getTargetReliabilityIndex()
print("Optimal design           :", design)
print("Analytic design          :", ot.Normal().computeQuantile(0.1)[0])
print("Target reliability index :", targetIndex)
print("Reliability index        :", algo.getReliabilityIndex())
print("Approximate probability  :", algo.getApproximateProbability())
print("Physical shift           :", algo.getShift())
print("Status                   :", result.getStatusMessage())

# %%
# Plot the objective and the optimal design.  The vertical line marks the
# SORA solution, which coincides with the target quantile of the distribution.
xMin = -2.5
xMax = 0.5
abscissas = [xMin + i * (xMax - xMin) / 200.0 for i in range(201)]
graph = ot.Graph("SORA on a linear limit state", "x", "f(x)", True, "topright")
objective = ot.Curve(ot.Sample([[x, x * x] for x in abscissas]))
objective.setColor("blue")
objective.setLegend("objective")
graph.add(objective)
optimum = ot.Curve(
    ot.Point([design, design]), ot.Point([0.0, design * design])
)
optimum.setColor("red")
optimum.setLegend("SORA optimum")
graph.add(optimum)
graph.setLegendPosition("topright")
view = otv.View(graph)
