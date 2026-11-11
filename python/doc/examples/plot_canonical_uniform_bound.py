r"""
Canonicalizing a robust problem with an optimized Uniform bound
===============================================================

This example illustrates the canonical construction on a non-regular model:
a Uniform distribution whose upper support bound is a decision variable.

Problem statement
-----------------

The random variable is :math:`\Theta \sim \mathcal{U}(0,b)`.  The decisions
are a setpoint :math:`x` and the upper bound :math:`b`:

.. math::

    \begin{aligned}
    & \underset{x, b}{\text{minimize}}
    & & \mathbb{E}\left[(\Theta-x)^2\right] \\
    & \text{subject to}
    & & \mathbb{P}\left(\Theta \ge 0.3\right) \ge 0.9, \\
    & & & x \in [0,5], \qquad b \in [0.1,5].
    \end{aligned}

The lower bound is fixed at zero, so the active ``Uniform`` parameter is the
upper bound only, at index ``1`` in the full parameter :math:`(a,b)`.

Why this model is non-regular
-----------------------------

The support :math:`[0,b]` depends on the parameter.  A robust measure such as
:math:`\mathbb{E}[(\Theta-x)^2]` therefore cannot be differentiated by simply
differentiating its integrand over a fixed interval: the moving endpoint
contributes a boundary term.  The canonical construction removes this
difficulty by fixing the reference distribution.  Here the canonicalizer
selects :math:`\Xi \sim \mathcal{N}(0,1)` and transports it with

.. math::

    T_b(\xi) = b\,\Phi(\xi),

so the image support is :math:`[0,b]` and the parameter only appears inside
the transformation.  The canonical constraint is evaluated over the fixed
reference :math:`\Xi`, which makes the problem regular.

Analytical solution
-------------------

For :math:`\Theta \sim \mathcal{U}(0,b)`,

.. math::

    \mathbb{E}\left[(\Theta-x)^2\right]
    = \frac{b^2}{12} + \left(\frac{b}{2}-x\right)^2,

which is minimized at :math:`x=b/2` with value :math:`b^2/12`.  The chance
constraint is

.. math::

    \mathbb{P}(\Theta \ge 0.3) = 1-\frac{0.3}{b} \ge 0.9,
    \qquad b \ge 0.3,

so it requires :math:`b \ge 3`.  The objective increases with :math:`b`, hence
the optimum is :math:`b=3`, :math:`x=1.5`, with objective
:math:`3^2/12=0.75` and probability exactly :math:`0.9`.
"""

# %%
import openturns as ot
import openturns.viewer as otv
import otrobopt


# %%
# Build the parametric model.  The initial distribution is
# :math:`\mathcal{U}(0,1)`; the upper bound will move during the
# optimization.
distribution = ot.Uniform(0.0, 1.0)
loss = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta"], ["(theta - x)^2"]), [1], [0.0]
)
threshold = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta"], ["theta - 0.3 + 0.0 * x"]), [1], [0.0]
)

objective = otrobopt.MeanMeasure(loss, distribution)
constraint = otrobopt.JointChanceMeasure(
    threshold, distribution, ot.GreaterOrEqual(), 0.9
)
problem = otrobopt.RobustOptimizationProblem(objective, constraint)
problem.setBounds(ot.Interval([0.0], [5.0]))

# %%
# Canonicalize.  The active parameter is the upper bound ``b``, at index
# ``1`` of the ``Uniform`` parameter ``(a, b)``.
options = otrobopt.CanonicalizationOptions()
options.setActiveParameter([1])
options.setParameterBounds(ot.Interval([0.1], [5.0]))
options.setParameterStart([1.0])

canonicalizer = otrobopt.RobustOptimizationProblemCanonicalizer(problem, options)
canonicalProblem = canonicalizer.getCanonicalProblem()
startingPoint = canonicalizer.getCanonicalStartingPoint()

print("Original design dimension       :", problem.getDimension())
print("Canonical dimension             :", canonicalProblem.getDimension())
print("Active parameter indices        :", canonicalizer.getActiveParameter())
print("Standard reference space        :", canonicalizer.usesStandardSpace())
print("Reference distribution          :", canonicalizer.getReferenceDistribution())
print("Canonical starting point        :", startingPoint)
print(
    "Canonical objective at start    :",
    canonicalizer.getCanonicalObjectiveFunction()(startingPoint),
)
print(
    "Canonical constraint at start   :",
    canonicalizer.getCanonicalConstraintFunction()(startingPoint),
)

# %%
# Solve the canonical problem.
solver = ot.Cobyla()
solver.setProblem(canonicalProblem)
solver.setStartingPoint(startingPoint)
solver.setMaximumIterationNumber(40)
solver.setCheckStatus(False)
solver.run()

result = solver.getResult()
optimum = result.getOptimalPoint()
print()
print("Status                          :", result.getStatusMessage())
print("Optimal canonical point         :", optimum)
print("Optimal canonical objective     :", result.getOptimalValue())
print(
    "Canonical constraint at optimum :",
    canonicalizer.getCanonicalConstraintFunction()(optimum),
)

# %%
# Compare with the analytical solution.  The canonical chance constraint is
# :math:`\Phi^{-1}(p)-\Phi^{-1}(0.9)`, so it vanishes when the probability
# reaches its target, independently of the value of the objective.
xOptimum = optimum[0]
bOptimum = optimum[1]
probability = 1.0 - 0.3 / bOptimum

xAnalytic = 1.5
bAnalytic = 3.0
objectiveAnalytic = 0.75

print()
print("Analytical x                    :", xAnalytic)
print("Analytical b                    :", bAnalytic)
print("Analytical objective            :", objectiveAnalytic)
print("x error                         :", abs(xOptimum - xAnalytic))
print("b error                         :", abs(bOptimum - bAnalytic))
print(
    "Objective error                 :",
    abs(result.getOptimalValue()[0] - objectiveAnalytic),
)
print("Probability at optimum          :", probability)

# %%
# Show the moving support explicitly, at the analytical bound.  The reference
# quantile of order 0.1 is mapped to the physical threshold 0.3, which is
# exactly the complement of the target probability.
reference = canonicalizer.getReferenceDistribution()
xiThreshold = reference.computeQuantile(0.1)[0]
print()
print("Reference quantile of order 0.1 :", xiThreshold)
print("T_b(xi) at that quantile        :", bAnalytic * reference.computeCDF(xiThreshold))
print("T_b(0) (median image)           :", bAnalytic * reference.computeCDF(0.0))
print("T_b(Phi^-1(0.9))                :", bAnalytic * 0.9)

# %%
# Quantify the missing boundary term in the original parameterization.  The
# probability is
#
# .. math::
#
#     P(b) = 1-\frac{0.3}{b} = \int_{0.3}^{b} \frac{1}{b}\,d\theta,
#     \qquad b \ge 0.3.
#
# At :math:`b=3` its derivative is :math:`0.3/b^2`.  Differentiating the
# integrand over the fixed interval :math:`[0.3,b]` gives the density term
# :math:`-(b-0.3)/b^2`; the moving upper endpoint adds the flux
# :math:`1/b`.  The two terms combine into the true derivative, which is why
# a naive fixed-support sensitivity is incorrect.
densityTerm = -(bAnalytic - 0.3) / bAnalytic**2
boundaryTerm = 1.0 / bAnalytic
probabilityDerivative = 0.3 / bAnalytic**2
print()
print("Probability derivative          :", probabilityDerivative)
print("  density term (fixed support)  :", densityTerm)
print("  moving-boundary flux          :", boundaryTerm)
print("  sum                           :", densityTerm + boundaryTerm)

# The canonical chance constraint applies the monotone probit transform, so
# its zero is unchanged while its slope is scaled by 1/phi(Phi^-1(0.9)).
targetQuantile = reference.computeQuantile(0.9)[0]
probitFactor = 1.0 / reference.computePDF(targetQuantile)
print("Canonical constraint derivative :", probabilityDerivative * probitFactor)

# %%
# Plot the objective along the optimal setpoints :math:`x=b/2`.  The
# unconstrained objective :math:`b^2/12` decreases with :math:`b`, while the
# chance constraint makes :math:`b \ge 3` feasible.
bValues = [0.1 + 4.9 * i / 200.0 for i in range(201)]
objectiveGraph = ot.Graph(
    "Objective along the optimal setpoint", "b", "expected squared error", True, "topright"
)
objectiveCurve = ot.Curve(
    ot.Sample([[b, b**2 / 12.0] for b in bValues])
)
objectiveCurve.setColor("blue")
objectiveCurve.setLegend("E[(Theta - b/2)^2]")
objectiveGraph.add(objectiveCurve)
feasibilityBoundary = ot.Curve([3.0, 3.0], [0.0, 2.1])
feasibilityBoundary.setColor("red")
feasibilityBoundary.setLineStyle("dashed")
feasibilityBoundary.setLegend("b = 3 (chance constraint)")
objectiveGraph.add(feasibilityBoundary)
optimumCloud = ot.Cloud(ot.Sample([[3.0, 0.75]]))
optimumCloud.setColor("red")
optimumCloud.setPointStyle("fsquare")
optimumCloud.setLegend("analytical optimum")
objectiveGraph.add(optimumCloud)
objectiveGraph.setLegendPosition("topright")
view = otv.View(objectiveGraph)

# %%
# Plot the canonical chance constraint.  It is negative for :math:`b<3` and
# becomes positive for :math:`b>3`, with a simple zero at the analytical
# optimum.
constraintGraph = ot.Graph(
    "Canonical chance constraint",
    "b",
    "Phi^-1(P) - Phi^-1(0.9)",
    True,
    "topright",
)
constraintPoints = []
for i in range(201):
    b = 0.31 + 4.69 * i / 200.0
    probabilityValue = 1.0 - 0.3 / b
    probitValue = reference.computeQuantile(probabilityValue)[0]
    constraintPoints.append([b, probitValue - targetQuantile])
constraintCurve = ot.Curve(ot.Sample(constraintPoints))
constraintCurve.setColor("blue")
constraintCurve.setLegend("canonical constraint")
constraintGraph.add(constraintCurve)
zeroLine = ot.Curve([0.1, 5.0], [0.0, 0.0])
zeroLine.setColor("black")
zeroLine.setLineStyle("dashed")
zeroLine.setLegend("zero level")
constraintGraph.add(zeroLine)
constraintCloud = ot.Cloud(ot.Sample([[3.0, 0.0]]))
constraintCloud.setColor("red")
constraintCloud.setPointStyle("fsquare")
constraintCloud.setLegend("analytical optimum")
constraintGraph.add(constraintCloud)
constraintGraph.setLegendPosition("bottomright")
view = otv.View(constraintGraph)
