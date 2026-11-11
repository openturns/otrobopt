r"""
Canonicalizing a robust problem over a correlated normal distribution
======================================================================

This example illustrates the canonical construction of
:class:`~otrobopt.RobustOptimizationProblemCanonicalizer` on a bivariate
normal model whose marginal standard deviations and correlation are decision
variables.

Problem statement
-----------------

The random vector is

.. math::

    \Theta \sim \mathcal{N}(\mu, \Sigma), \qquad
    \mu = (2, 2), \qquad
    \Sigma =
    \begin{pmatrix}
    \sigma_1^2 & \rho\sigma_1\sigma_2\\
    \rho\sigma_1\sigma_2 & \sigma_2^2
    \end{pmatrix}.

The means are fixed.  The decisions are the design vector
:math:`x=(x_1,x_2)` and the distribution parameter
:math:`s=(\sigma_1,\sigma_2,\rho)`.  The robust problem is

.. math::

    \begin{aligned}
    & \underset{x, s}{\text{minimize}}
    & & \mathbb{E}\left[10(\Theta_1-x_1)^2+(\Theta_2-x_2)^2\right] \\
    & \text{subject to}
    & & \mathbb{E}\left[(\Theta_1-\Theta_2)^2\right] \ge 0.6, \\
    & & & x_1,x_2 \in [0,4], \\
    & & & \sigma_1,\sigma_2 \in [0.2,1.5], \qquad \rho \in [-0.8,0.8].
    \end{aligned}

The objective is an asymmetric quality loss: a deviation of the first
component costs ten times more than a deviation of the second one.  The
constraint requires a minimum expected squared separation between the two
components, a simple detectability requirement.

The canonical construction replaces the moving distribution
:math:`\mathcal{N}(\mu,\Sigma_s)` by a fixed reference distribution
:math:`\Xi`, here :math:`\mathcal{N}(0,I_2)`, and rebuilds the measures as

.. math::

    P(x,s;\xi) = f\bigl(x; T_s(\xi)\bigr),

where :math:`T_s` transports :math:`\Xi` to :math:`\mathcal{N}(\mu,\Sigma_s)`.
The canonical decision vector is therefore :math:`(x,s)`.

OpenTURNS stores the parameter of a bivariate ``Normal`` as

.. math::

    (\mu_0,\sigma_0,\mu_1,\sigma_1,R_{1,0}),

so fixing the means and activating the two standard deviations and the
correlation corresponds to the active indices ``[1, 3, 4]``.

Analytical solution
-------------------

At :math:`x=(2,2)` the objective reduces to
:math:`10\sigma_1^2+\sigma_2^2`, while the constraint reads

.. math::

    \sigma_1^2+\sigma_2^2-2\rho\sigma_1\sigma_2 \ge 0.6.

For a fixed cost, a negative correlation increases the separation, so the
optimizer drives :math:`\rho` to its lower bound :math:`-0.8`.  The
expensive component then stays at its lower standard-deviation bound
:math:`\sigma_1=0.2`, and the cheap component supplies the remaining
separation:

.. math::

    \sigma_2^2+1.6(0.2)\sigma_2+0.2^2-0.6=0
    \quad\Longrightarrow\quad
    \sigma_2 \approx 0.6052.

The optimal objective is :math:`10(0.2)^2+(0.6052)^2\approx 0.7663`, and the
constraint is active.
"""

# %%
import openturns as ot
import openturns.viewer as otv
import otrobopt


def cheap_quadrature(measure):
    """Keep the gallery build fast without changing the optimum."""
    measure.setIntegrationAlgorithm(
        ot.IteratedQuadrature(
            ot.GaussKronrod(
                3, 1.0e-2, ot.GaussKronrodRule(ot.GaussKronrodRule.G7K15)
            )
        )
    )
    return measure


# %%
# Build the parametric model.  The means are fixed by the parametric
# function: the realizations enter as the last two inputs.
mean = [2.0, 2.0]
correlation = ot.CorrelationMatrix(2)
correlation[0, 1] = 0.0
distribution = ot.Normal(mean, [1.0, 1.0], correlation)

loss = ot.ParametricFunction(
    ot.SymbolicFunction(
        ["x1", "x2", "t1", "t2"],
        ["10 * (t1 - x1)^2 + (t2 - x2)^2"],
    ),
    [2, 3],
    [0.0, 0.0],
)
separation = ot.ParametricFunction(
    ot.SymbolicFunction(
        ["x1", "x2", "t1", "t2"],
        ["(t1 - t2)^2 - 0.6 + 0.0 * x1 + 0.0 * x2"],
    ),
    [2, 3],
    [0.0, 0.0],
)

objective = cheap_quadrature(otrobopt.MeanMeasure(loss, distribution))
constraint = cheap_quadrature(otrobopt.MeanMeasure(separation, distribution))
problem = otrobopt.RobustOptimizationProblem(objective, constraint)
problem.setBounds(ot.Interval([0.0, 0.0], [4.0, 4.0]))

# %%
# Canonicalize.  The active parameter is expressed in the order of the full
# ``Normal`` parameter, and the bounds and starting point follow the same
# order: here ``[sigma_0, sigma_1, R_1_0]``.
options = otrobopt.CanonicalizationOptions()
options.setActiveParameter([1, 3, 4])
options.setParameterBounds(ot.Interval([0.2, 0.2, -0.8], [1.5, 1.5, 0.8]))
options.setParameterStart([0.5, 0.8, -0.2])

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
# Solve the canonical problem with Cobyla.  The canonical problem is a
# standard deterministic problem over a fixed reference distribution.
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
# Compare with the analytical solution and with a freshly parameterized
# distribution, which is the original problem evaluated at the optimum.
sigma1 = optimum[2]
sigma2 = optimum[3]
rho = optimum[4]
x1 = optimum[0]
x2 = optimum[1]

analyticSigma1 = 0.2
analyticRho = -0.8
analyticSigma2 = analyticRho * analyticSigma1 + (
    0.6 - (1.0 - analyticRho**2) * analyticSigma1**2
) ** 0.5
analyticObjective = 10.0 * analyticSigma1**2 + analyticSigma2**2

print()
print("Analytical sigma1               :", analyticSigma1)
print("Analytical sigma2               :", analyticSigma2)
print("Analytical rho                  :", analyticRho)
print("Analytical objective            :", analyticObjective)
print("Sigma1 error                    :", abs(sigma1 - analyticSigma1))
print("Sigma2 error                    :", abs(sigma2 - analyticSigma2))
print("Rho error                       :", abs(rho - analyticRho))

freshCorrelation = ot.CorrelationMatrix(2)
freshCorrelation[0, 1] = rho
freshDistribution = ot.Normal(mean, [sigma1, sigma2], freshCorrelation)
freshObjective = otrobopt.MeanMeasure(loss, freshDistribution)
freshConstraint = otrobopt.MeanMeasure(separation, freshDistribution)
print()
print("Fresh distribution              :", freshDistribution)
print("Fresh objective at optimum      :", freshObjective([x1, x2]))
print("Fresh constraint at optimum     :", freshConstraint([x1, x2]))

# %%
# Plot the objective along the active constraint.  For a fixed correlation,
# the constraint determines the smallest feasible standard deviation and the
# other standard deviation follows.  The curve is therefore parameterized by
# ``sigma2``, and its right endpoint is the cheapest feasible point for that
# correlation.  Lower correlations are uniformly better, and the best curve
# ends at the analytical optimum.  Positive correlations, not shown, are even
# more expensive because they force both standard deviations to grow.
graph = ot.Graph(
    "Objective along the active constraint",
    "sigma2",
    "expected loss",
    True,
    "topright",
)
colors = ["blue", "green", "orange"]
for rhoValue, color in zip([-0.8, -0.4, 0.0], colors):
    points = []
    for i in range(201):
        sigma2Value = 0.2 + 1.3 * i / 200.0
        discriminant = 0.6 - (1.0 - rhoValue**2) * sigma2Value**2
        if discriminant < 0.0:
            continue
        sigma1Value = rhoValue * sigma2Value + discriminant**0.5
        if sigma1Value < 0.2 or sigma1Value > 1.5:
            continue
        points.append([sigma2Value, 10.0 * sigma1Value**2 + sigma2Value**2])
    curve = ot.Curve(ot.Sample(points))
    curve.setColor(color)
    curve.setLegend("rho = {:+.1f}".format(rhoValue))
    graph.add(curve)

optimumCloud = ot.Cloud(ot.Sample([[analyticSigma2, analyticObjective]]))
optimumCloud.setColor("red")
optimumCloud.setPointStyle("fsquare")
optimumCloud.setLegend("analytical optimum")
graph.add(optimumCloud)
graph.setLegendPosition("topright")
view = otv.View(graph)
