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

Ground truth
------------

Let :math:`s=x_1+x_2`.  The two components are independent and each safe event
is :math:`\{\Theta_i \ge s\}`, so the exact joint probability is
:math:`(1-\Phi(s))^2`.  The exact boundary for the target :math:`0.9`
therefore satisfies :math:`1-\Phi(s)=\sqrt{0.9}`, i.e.
:math:`s=\Phi^{-1}(1-\sqrt{0.9})=-1.6322`.  The optimal design is symmetric,
:math:`x_1=x_2=s/2=-0.8161`, with objective :math:`2(s/2)^2=1.3320`.

The example compares both treatments with this ground truth.  It also counts
the objective and limit-state evaluations of each treatment.  The scalarized
FORM treatment is expected to underestimate the joint probability of this
two-component system and therefore to be non-conservative, while the
Bonferroni treatment is expected to be conservative.
"""

# %%
import openturns as ot
import openturns.viewer as otv
import otrobopt


def build_problem():
    """Return a fresh problem and its memoized objective and limit state."""
    objective = ot.MemoizeFunction(
        ot.SymbolicFunction(["x1", "x2"], ["x1^2 + x2^2"])
    )
    objective.disableCache()
    limitState = ot.MemoizeFunction(
        ot.ParametricFunction(
            ot.SymbolicFunction(
                ["x1", "x2", "t1", "t2"],
                ["t1 - x1 - x2", "t2 - x1 - x2"],
            ),
            [2, 3],
            [0.0, 0.0],
        )
    )
    limitState.disableCache()
    distribution = ot.JointDistribution([ot.Normal(), ot.Normal()])
    measure = otrobopt.JointChanceMeasure(
        limitState, distribution, ot.GreaterOrEqual(), 0.9
    )
    problem = otrobopt.RBDOProblem(objective, measure)
    return problem, objective, limitState


def solve(treatment):
    """Solve a fresh problem with the given joint-chance treatment."""
    problem, objective, limitState = build_problem()
    solver = ot.Cobyla()
    solver.setMaximumIterationNumber(500)
    algo = otrobopt.SORARobustOptimizationAlgorithm(problem, solver)
    algo.setJointChanceTreatment(treatment)
    algo.setStartingPoint([0.0, 0.0])
    algo.run()
    return algo, objective, limitState


# %%
# Solve the problem with both treatments and report the results.
algoExact, objectiveExact, limitStateExact = solve(
    otrobopt.SORARobustOptimizationAlgorithm.EXACT
)
algoBonferroni, objectiveBonferroni, limitStateBonferroni = solve(
    otrobopt.SORARobustOptimizationAlgorithm.BONFERRONI
)

results = [
    ("EXACT", algoExact, objectiveExact, limitStateExact),
    ("BONFERRONI", algoBonferroni, objectiveBonferroni, limitStateBonferroni),
]

for name, algo, objective, limitState in results:
    print("Treatment                :", name)
    print("Optimal design           :", algo.getResult().getOptimalPoint())
    print("Target reliability index :", algo.getTargetReliabilityIndex())
    print("Reliability index        :", algo.getReliabilityIndex())
    print("Shift norm               :", algo.getShift().norm())
    print("Objective evaluations    :", objective.getInputHistory().getSize())
    print("Limit-state evaluations  :", limitState.getInputHistory().getSize())
    print()

# %%
# Analytical ground truth.  The exact joint boundary and the exact optimal
# design are known in closed form for this symmetric system.
normal = ot.Normal()
exactSum = ot.DistFunc.qNormal(1.0 - 0.9**0.5)
exactDesign = exactSum / 2.0
exactObjective = 2.0 * exactDesign**2
print("Exact joint boundary     :", exactSum)
print("Exact design             :", exactDesign)
print("Exact objective          :", exactObjective)
print()

# %%
# Compare each treatment with the ground truth.  ``trueProbability`` is the
# exact joint probability at the returned design.  A treatment is feasible for
# the joint target only when this probability is at least 0.9.
for name, algo, _, _ in results:
    point = algo.getResult().getOptimalPoint()
    designSum = point[0] + point[1]
    trueProbability = (1.0 - normal.computeCDF(designSum)) ** 2
    print("Treatment                :", name)
    print("Design error             :", abs(point[0] - exactDesign))
    print("Objective error          :",
          abs(algo.getResult().getOptimalValue()[0] - exactObjective))
    print("True joint probability   :", trueProbability)
    print("Feasible for target 0.9  :", trueProbability >= 0.9)
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

trueCurve = ot.Curve(
    ot.Point(abscissas), ot.Point([exactSum - x for x in abscissas])
)
trueCurve.setColor("black")
trueCurve.setLineStyle("dotted")
trueCurve.setLegend("exact boundary")
graph.add(trueCurve)

graph.setLegendPosition("lower left")
view = otv.View(graph)
