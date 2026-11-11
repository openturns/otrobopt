r"""
SORA with two joint chance treatments
=====================================

This example illustrates the two explicit joint-chance treatments of
:class:`~otrobopt.SORARobustOptimizationAlgorithm` on a two-component system.

Problem statement
-----------------

.. math::

    \begin{aligned}
    & \underset{x_1, x_2}{\text{minimize}}
    & & x_1^2 + x_2^2 \\
    & \text{subject to}
    & & \mathbb{P}_{\Theta}(\Theta_1 - x_1 - x_2 \ge 0,\;
                            \Theta_2 - x_1 - x_2 \ge 0) \ge 0.9, \\
    & & & \Theta_1, \Theta_2 \sim \mathcal{N}(0, 1)
          \text{ independent}.
    \end{aligned}

The design dimension and the random dimension are both equal to two, as
required by the physical shift of SORA.

``EXACT`` replaces the joint event by the scalarized limit state
:math:`\min_j g_j(x;\Theta)` and uses the target reliability index
:math:`\Phi^{-1}(0.9)`.  This is a FORM approximation: it does not compute the
exact system probability.  ``BONFERRONI`` enforces each component with the
conservative target :math:`1-(1-0.9)/2=0.95`.

The two treatments therefore produce different boundaries in the design plane,
which is what the final figure shows.
"""

# %%
import openturns as ot
import openturns.viewer as otv
import otrobopt

# %%
# Build the two-component system.  The limit-state parameters are the
# realizations of the uncertain vector.
limitState = ot.ParametricFunction(
    ot.SymbolicFunction(
        ["x1", "x2", "t1", "t2"],
        ["t1 - x1 - x2", "t2 - x1 - x2"],
    ),
    [2, 3],
    [0.0, 0.0],
)
distribution = ot.JointDistribution([ot.Normal(), ot.Normal()])
measure = otrobopt.JointChanceMeasure(
    limitState, distribution, ot.GreaterOrEqual(), 0.9
)
problem = otrobopt.RBDOProblem(
    ot.SymbolicFunction(["x1", "x2"], ["x1^2 + x2^2"]), measure
)


def solve(treatment):
    solver = ot.Cobyla()
    solver.setMaximumIterationNumber(500)
    algo = otrobopt.SORARobustOptimizationAlgorithm(problem, solver)
    algo.setJointChanceTreatment(treatment)
    algo.setStartingPoint([0.0, 0.0])
    algo.run()
    return algo


# %%
# Solve the problem with both treatments and compare the results.
algoExact = solve(otrobopt.SORARobustOptimizationAlgorithm.EXACT)
algoBonferroni = solve(
    otrobopt.SORARobustOptimizationAlgorithm.BONFERRONI
)

for name, algo in [
    ("EXACT", algoExact),
    ("BONFERRONI", algoBonferroni),
]:
    print("Treatment                :", name)
    print("Optimal design           :", algo.getResult().getOptimalPoint())
    print("Target reliability index :", algo.getTargetReliabilityIndex())
    print("Reliability index        :", algo.getReliabilityIndex())
    print("Shift norm               :", algo.getShift().norm())
    print()

# %%
# Plot the two constraint boundaries and the optimal designs.  The boundary is
# :math:`x_1+x_2=-\Phi^{-1}(\alpha_t)`, where :math:`\alpha_t` is the target
# index of the selected treatment.
graph = ot.Graph(
    "Joint chance treatments of SORA", "x1", "x2", True, "lower left"
)
abscissas = [-1.8, 0.5]

for name, algo, color in [
    ("EXACT boundary", algoExact, "blue"),
    ("BONFERRONI boundary", algoBonferroni, "red"),
]:
    level = -algo.getTargetReliabilityIndex()
    ordinates = [level - x for x in abscissas]
    curve = ot.Curve(ot.Point(abscissas), ot.Point(ordinates))
    curve.setColor(color)
    curve.setLineStyle("dashed")
    curve.setLegend(name)
    graph.add(curve)

    point = algo.getResult().getOptimalPoint()
    cloud = ot.Cloud(ot.Sample([point]))
    cloud.setColor(color)
    cloud.setPointStyle("circle")
    cloud.setLegend(name.replace("boundary", "optimum"))
    graph.add(cloud)

graph.setLegendPosition("lower left")
view = otv.View(graph)
