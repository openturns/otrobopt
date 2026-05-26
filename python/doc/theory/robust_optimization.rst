.. _robust_optimization_theory:

Robust optimization
===================

This section describes the theoretical framework behind the robust
optimisation features of the platform.

General formulation
-------------------

Let :math:`J(x, \theta)` be a cost function that depends on
a design variable :math:`x \in \mathbb{R}^{n_x}` and a random parameter
:math:`\theta \sim \mathcal{D}` with known distribution.
Let :math:`G(x, \theta)` be a constraint function.

The robust optimization problem reads:

.. math::

    \begin{aligned}
    & \underset{x}{\text{minimize}}
    & & \rho_{\mathcal{D}}[J(x, \cdot)]
    \\
    & \text{subject to}
    & & \lambda_{\mathcal{D}}[G(x, \cdot)] \geq 0
    \end{aligned}

where :math:`\rho_{\mathcal{D}}` is a *robustness measure* on the objective
and :math:`\lambda_{\mathcal{D}}` a *reliability measure* on the constraint.
Both are statistical functionals evaluated over the random parameter
:math:`\theta`.


Robustness measures
-------------------

These measures quantify the *performance* of a design :math:`x` in
the presence of uncertainty. They are used as the optimisation objective.

Mean
^^^^

The **MeanMeasure** computes the expectation of the objective:

.. math::

    \rho_{\mathcal{D}}[J(x, \cdot)]
    = \mathbb{E}_{\mathcal{D}}[J(x, \theta)]
    = \int J(x, \theta) \, p(\theta) \, d\theta

where :math:`p` is the probability density function of :math:`\theta`.
This is the standard *risk-neutral* approach.

For discrete distributions, the integral reduces to a weighted sum:

.. math::

    \rho_{\mathcal{D}}[J(x, \cdot)]
    = \sum_{i=1}^n J(x, \theta_i) \, p_i

Variance
^^^^^^^^

The **VarianceMeasure** computes the variance of each output component:

.. math::

    \rho_{\mathcal{D}}[J(x, \cdot)]
    = \mathbb{V}_{\mathcal{D}}[J(x, \theta)]
    = \mathbb{E}_{\mathcal{D}}[J(x, \theta)^2]
      - \mathbb{E}_{\mathcal{D}}[J(x, \theta)]^2

This measure can be used for *risk-averse* optimisation.

Mean-Standard deviation tradeoff
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The **MeanStandardDeviationTradeoffMeasure** computes a convex combination
of the mean and the standard deviation for each output component:

.. math::

    \rho_{\mathcal{D}}[J(x, \cdot)]_k
    = (1 - \alpha_k) \, \mu_k(x) + \alpha_k \, \sigma_k(x)

where :math:`\mu_k = \mathbb{E}[J_k]`, :math:`\sigma_k = \sqrt{\mathbb{V}[J_k]}`
and :math:`\alpha_k \in [0, 1]` is a trade-off coefficient.

Special cases:

* :math:`\alpha_k = 0` gives the pure mean (risk-neutral),
* :math:`\alpha_k = 1` gives the pure standard deviation,
* :math:`\alpha_k = 0.5` gives equal weight to both.

Worst-case
^^^^^^^^^^

The **WorstCaseMeasure** computes the extremal value of the function over
the support of the distribution:

.. math::

    \rho_{\mathcal{D}}[J(x, \cdot)]
    = \inf_{\theta \in \operatorname{Supp}(\mathcal{D})} J(x, \theta)

for minimization, or the supremum for maximisation.

For continuous distributions, this is a nested optimisation problem
solved with an inner solver (default: TNC). For discrete distributions,
the support is scanned exhaustively.

Quantile
^^^^^^^^

The **QuantileMeasure** computes the :math:`\alpha`-quantile of the
output distribution:

.. math::

    \rho_{\mathcal{D}}[J(x, \cdot)]
    = \inf \left\{ s \in \mathbb{R} \;
      : \; \mathbb{P}_{\mathcal{D}}(J(x, \theta) \leq s) \geq \alpha \right\}

This requires solving :math:`F(s) = \alpha` where
:math:`F(s) = \mathbb{P}(J(x, \theta) \leq s)` is the cumulative distribution
function of the output. A Brent root-finding algorithm is used.

This measure is restricted to scalar (single-output) functions.


Reliability measures
--------------------

These measures quantify the *feasibility* of a design :math:`x` under
uncertainty. They are used as constraints in the optimisation problem.

Joint chance constraint
^^^^^^^^^^^^^^^^^^^^^^^

The **JointChanceMeasure** computes the probability that **all** constraint
components are satisfied simultaneously, shifted by a target level
:math:`\alpha`:

.. math::

    \lambda_{\mathcal{D}}[G(x, \cdot)]
    = \mathbb{P}_{\mathcal{D}}
      \left( \bigcap_{k=1}^{n_g} \{ G_k(x, \theta) \geq 0 \} \right) - \alpha

The integral is:

.. math::

    \lambda = \int \left( \prod_{k=1}^{n_g}
      \mathbf{1}_{\{ G_k(x, \theta) \geq 0 \}} \right)
      p(\theta) \, d\theta \; - \; \alpha

When a comparison operator other than *Greater* is used, the sign is
adjusted so that the constraint :math:`\lambda \geq 0` corresponds to
the desired reliability level.

Individual chance constraint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The **IndividualChanceMeasure** computes the probability that **each**
constraint component is satisfied **individually**, shifted by per-component
target levels:

.. math::

    \lambda_{\mathcal{D}}[G(x, \cdot)]_k
    = \mathbb{P}_{\mathcal{D}}( G_k(x, \theta) \geq 0 ) - \alpha_k

for :math:`k = 1, \dots, n_g`. The constraint
:math:`\lambda_{\mathcal{D}} \geq 0` is interpreted component-wise.


Aggregated measure
------------------

The **AggregatedMeasure** concatenates the outputs of several measures
into a single vector-valued measure:

.. math::

    \mathcal{M}(x) = \left[ \rho_1(x), \rho_2(x), \dots, \rho_m(x) \right]

This makes it possible to combine, for instance, a robustness measure on
the objective with a reliability measure on the constraint into a single
measure evaluation. It is used internally by the
:class:`~otrobopt.RobustOptimizationProblem`.


Robust optimization problem
---------------------------

The :class:`~otrobopt.RobustOptimizationProblem` class assembles the
robustness and reliability measures into a mathematical program:

.. math::

    \begin{aligned}
    & \underset{x}{\text{minimize}}
    & & \rho_{\mathcal{D}}[J(x, \cdot)]
    \\
    & \text{subject to}
    & & \lambda_{\mathcal{D}}[G(x, \cdot)] \geq 0
    \end{aligned}

Either the robustness measure or the reliability measure (or both) can
be omitted, in which case the corresponding component remains
deterministic. Concretely:

* If :math:`\rho` is omitted, the original deterministic objective is used.
* If :math:`\lambda` is omitted, the original deterministic constraint is used.
* If both are provided, they must share the same uncertainty distribution.

The problem may also include deterministic bound constraints via
:meth:`~otrobopt.RobustOptimizationProblem.setBounds`.


Discretisation of measures
--------------------------

**MeasureFactory** converts a measure with a continuous distribution into
a measure with a discrete (empirical) distribution:

.. math::

    \mathcal{D} \;\longrightarrow\; \sum_{i=1}^N \omega_i \, \delta_{\theta_i}

where :math:`\{\theta_i\}` and :math:`\{\omega_i\}` are generated by a
:class:`~openturns.WeightedExperiment` (e.g. Monte Carlo, LHS,
Sobol' sequence). The discrete measure then replaces the integral by a
finite sum, making the optimisation problem tractable via standard
deterministic solvers.


Sequential Monte Carlo algorithm
--------------------------------

The :class:`~otrobopt.SequentialMonteCarloRobustAlgorithm` is the
principal solver. It iteratively refines the discretisation and re-optimises
until convergence.

Algorithm
^^^^^^^^^

.. math::

    \begin{array}{l}
    \textbf{Given: } N_0 \text{ (initial sample size)}, \; \varepsilon
    \text{ (convergence tolerance)} \\
    \textbf{Initialize: } N \leftarrow N_0 \\
    \textbf{Repeat:} \\
    \qquad \Delta N \leftarrow \text{samplingSizeIncrement}(N) \\
    \qquad \text{Draw } \Delta N \text{ new samples } \{\theta_i\}
    \text{ from } \mathcal{D} \\
    \qquad \text{Append to existing sample set; } N \leftarrow N + \Delta N \\
    \qquad \text{Discretise the measures via MeasureFactory} \\
    \qquad \text{Solve the resulting deterministic problem from } x_{k-1} \\
    \qquad \text{Let } x_k \text{ be the new optimum} \\
    \qquad \text{If } \|x_k - x_{k-1}\| < \varepsilon \text{, stop}
    \end{array}

Key aspects:

* **Sample size**: At each iteration, new samples are added to improve
  the accuracy of the discretisation. The increment function can be
  configured (default: add :math:`N` new samples each iteration).
* **Multi-start**: On the first iteration, an optional multi-start search
  (using LHS over the bounds) helps find a promising starting point.
* **Convergence**: The algorithm stops when the design point change falls
  below the absolute error tolerance.
* **Solver tolerance**: The inner deterministic solver tolerance is
  tightened as :math:`\varepsilon_k = C / \sqrt{N}` where :math:`C` is a
  configurable factor.

This sequential approach avoids committing to a fixed discretisation
level *a priori*: the approximation improves as the optimisation
progresses, leading to efficient and robust convergence.
