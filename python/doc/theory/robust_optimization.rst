.. _robust-optimization-measures:

Robust optimization measures
============================

.. currentmodule:: otrobopt

This page is the catalogue of the robustness and reliability measures
provided by otrobopt.  It states the definition, the output dimension and the
intended use of each measure, together with the discretization and solver
interfaces.  The mathematical contract, the invariance theorem and the
canonical transformation are stated in :doc:`theory`; this page complements it
with the measure-by-measure reference.

General formulation
-------------------

Let :math:`J(x, \theta)` be a cost function that depends on a design vector
:math:`x \in \Rset^{n_x}` and on a realization :math:`\theta` of the random
vector :math:`\Theta \in \Rset^{n_\theta}` with distribution :math:`\cD`.  Let
:math:`G(x, \theta)` be a constraint function.  The robust optimization
problem reads

.. math::

    \begin{aligned}
    & \min_{x \in \mathcal{X}}
    & & \rho_{\cD}\bigl[J(x, \cdot)\bigr] \\
    & \text{subject to}
    & & \lambda_{\cD}\bigl[G(x, \cdot)\bigr] \ge 0,
    \end{aligned}

where :math:`\rho_{\cD}` is a *robustness measure* on the objective and
:math:`\lambda_{\cD}` is a *reliability measure* on the constraint.  Both are
statistical functionals of the random variable obtained by freezing
:math:`x`.  The feasible set :math:`\mathcal{X}` carries the deterministic
bounds and constraints of the problem.

The measures are implemented as subclasses of
:class:`~otrobopt.MeasureEvaluation`.  A measure evaluates its function at the
design point and integrates, samples or optimizes over the parameter
:math:`\theta`, which is the OpenTURNS *parameter* of the function.  The
design vector :math:`x` is the measure input.

Robustness measures
-------------------

Robustness measures quantify the performance of a design under uncertainty.
They are used as the objective, and therefore must be scalar-valued.
:class:`~otrobopt.RobustOptimizationProblem` rejects a vector-valued robust
objective.

Mean
^^^^

:class:`~otrobopt.MeanMeasure` computes the expectation of the objective,

.. math::

    \rho_{\cD}\bigl[J(x, \cdot)\bigr]
    = \Expect{J(x, \Theta)}
    = \int_{\Rset^{n_\theta}} J(x, \theta)\, \cD(\mathrm{d}\theta).

This is the risk-neutral approach.  For a discrete distribution the integral
reduces to a weighted sum,

.. math::

    \rho_{\cD}\bigl[J(x, \cdot)\bigr]
    = \sum_{i=1}^{N} J(x, \theta_i)\, p_i,

where :math:`p_i` is the probability of the atom :math:`\theta_i`.  The mean
is defined for a vector-valued function as well; the scalar restriction comes
from the robust problem, not from the measure.

The integral is evaluated with a density threshold: a density below
``getPDFThreshold()`` (``2e-16`` by default) is treated as zero.  The
threshold applies to the distribution of the measure, so after a
canonicalization it applies to the reference density.  Raising it can discard
a region that the original integral would have kept, in particular when the
reference distribution has a different scale from the original one.

Variance
^^^^^^^^

:class:`~otrobopt.VarianceMeasure` computes the variance of every output
component,

.. math::

    \rho_{\cD}\bigl[J(x, \cdot)\bigr]
    = \Var{J(x, \Theta)}
    = \Expect{J(x, \Theta)^2} - \Expect{J(x, \Theta)}^2.

It is used for risk-averse optimization.  The variance is a scalar only when
the function is scalar-valued.

Mean-standard deviation tradeoff
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:class:`~otrobopt.MeanStandardDeviationTradeoffMeasure` computes a convex
combination of the mean and the standard deviation,

.. math::

    \rho_{\cD}\bigl[J(x, \cdot)\bigr]_k
    = (1 - \alpha_k)\, \mu_k(x) + \alpha_k\, \sigma_k(x),

where :math:`\mu_k = \Expect{J_k(x, \Theta)}`,
:math:`\sigma_k = \sqrt{\Var{J_k(x, \Theta)}}` and
:math:`\alpha_k \in [0,1]` is a trade-off coefficient.  The special values are
:math:`\alpha_k = 0` for the pure mean, :math:`\alpha_k = 1` for the pure
standard deviation and :math:`\alpha_k = 1/2` for equal weights.

Worst case
^^^^^^^^^^

:class:`~otrobopt.WorstCaseMeasure` computes the extremal value of the
function over the support of the distribution,

.. math::

    \rho_{\cD}\bigl[J(x, \cdot)\bigr]
    = \inf_{\theta \in \supp{\cD}} J(x, \theta)

for minimization, and the supremum for maximization.  When the user sets an
optimization domain with ``setOptimizationDomain``, the extremum is taken over
that domain.  Otherwise a continuous distribution is searched over
``distribution.getRange()``; an unbounded range therefore requires an explicit
domain when the measure is canonicalized (the domain policy is described in
:doc:`theory`).  For continuous distributions the inner problem is solved by a
nested optimizer (default: TNC); for discrete distributions the support is
scanned exhaustively.

The domain stored by a worst-case measure is expressed in the coordinates of
the original uncertain vector :math:`\theta`.  When the distribution parameter
is frozen, the canonicalization keeps these coordinates and preserves the
domain.  When the parameter is optimized, the worst-case variable becomes the
reference realization and the stored domain has no image in general: the
canonicalizer rejects the configuration instead of silently dropping the
restriction.  Provide the domain in the reference space with
``CanonicalizationOptions.setWorstCaseDomain``, or freeze the parameter with
``setOptimizeParameter(False)``.

Quantile
^^^^^^^^

:class:`~otrobopt.QuantileMeasure` computes the :math:`\alpha`-quantile of the
output distribution,

.. math::

    \rho_{\cD}\bigl[J(x, \cdot)\bigr]
    = \inf\Bigl\{ t \in \Rset :
      \Prob{J(x, \Theta) \le t} \ge \alpha \Bigr\}.

The implementation solves :math:`F(t) = \alpha` by Brent root finding when
:math:`F` is continuous, where :math:`F` is the distribution function of
:math:`J(x, \Theta)`.  For a discrete distribution the measure builds the
finite discrete distribution of the output and calls its quantile function
directly, because a discrete distribution function need not attain
:math:`\alpha`.  The measure is restricted to scalar functions.

Reliability measures
--------------------

Reliability measures quantify the feasibility of a design under uncertainty.
They are used as constraints.  The otrobopt convention is
:math:`g(x) \ge 0` for an inequality constraint, so a reliability requirement
:math:`\Prob{G(x, \Theta) \ge 0} \ge \alpha` must be expressed with
:class:`~openturns.GreaterOrEqual`.

Joint chance constraint
^^^^^^^^^^^^^^^^^^^^^^^

:class:`~otrobopt.JointChanceMeasure` computes the probability that **all**
constraint components are satisfied simultaneously, mapped through the probit
transform :math:`\Phi^{-1}` and shifted by the target level :math:`\alpha`:

.. math::

    \lambda_{\cD}\bigl[G(x, \cdot)\bigr]
    = \Phi^{-1}\!\left(
      \Prob{\bigcap_{k=1}^{n_g} \{G_k(x, \Theta) \ge 0\}}
      \right) - \Phi^{-1}(\alpha).

The probability is evaluated as

.. math::

    P(x) = \int_{\Rset^{n_\theta}}
      \left(\prod_{k=1}^{n_g}
      \mathbf{1}_{\{G_k(x, \theta) \ge 0\}}\right)
      \cD(\mathrm{d}\theta).

The measure always integrates the event :math:`G(x, \Theta) \ge 0`; the
comparison operator only selects the sign of the returned scalar.  With
:class:`~openturns.GreaterOrEqual` the value is

.. math::

    \lambda = \Phi^{-1}\bigl(P(x)\bigr) - \Phi^{-1}(\alpha),

so the constraint :math:`\lambda \ge 0` is equivalent to
:math:`P(x) \ge \alpha`.  With :class:`~openturns.Less` the value is

.. math::

    \lambda = \Phi^{-1}(\alpha) - \Phi^{-1}\bigl(P(x)\bigr),

which represents :math:`P(x) \le \alpha`, equivalently
:math:`\Prob{G(x, \Theta) < 0} \ge 1 - \alpha`.  The ``Less`` form is therefore
the complement form, not a different event.  A reliability requirement
:math:`\Prob{G(x, \Theta) \ge 0} \ge \alpha` uses ``GreaterOrEqual``.

The probit transform stretches the probability scale near zero and one, which
improves the conditioning of the measure for gradient-based optimization.  The
measure is scalar-valued even when the limit state is vector-valued.

Individual chance constraint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:class:`~otrobopt.IndividualChanceMeasure` computes the probability that
**each** constraint component is satisfied **individually**, mapped through
the probit transform and shifted by per-component target levels,

.. math::

    \lambda_{\cD}\bigl[G(x, \cdot)\bigr]_k
    = \Phi^{-1}\!\left(
      \Prob{G_k(x, \Theta) \ge 0}
      \right) - \Phi^{-1}(\alpha_k),

for :math:`k = 1, \dots, n_g`.  The output is a vector with one entry per
constraint component and the constraint :math:`\lambda_{\cD} \ge 0` is
interpreted componentwise.  As for the joint measure, the operator selects the
sign of each component and the integrated event remains
:math:`G_k(x, \Theta) \ge 0`.

Aggregated measure
------------------

:class:`~otrobopt.AggregatedMeasure` concatenates the outputs of several
measures into a single vector-valued measure,

.. math::

    \mathcal{M}(x) = \bigl[\rho_1(x), \rho_2(x), \dots, \rho_m(x)\bigr].

It is used internally by :class:`~otrobopt.RobustOptimizationProblem` to
assemble the objective and the constraints into one evaluation.  The children
share the same distribution and the same parameter vector.  A mixed set of
distributions is rejected by the problem when the measures differ at the top
level.  The canonicalizer also rejects a mixed set of distributions inside an
aggregate.

Robust optimization problem
---------------------------

:class:`~otrobopt.RobustOptimizationProblem` assembles the robustness and
reliability measures into the mathematical program

.. math::

    \begin{aligned}
    & \min_{x \in \mathcal{X}}
    & & \rho_{\cD}\bigl[J(x, \cdot)\bigr] \\
    & \text{subject to}
    & & \lambda_{\cD}\bigl[G(x, \cdot)\bigr] \ge 0.
    \end{aligned}

Either measure can be omitted.  If :math:`\rho` is omitted, the original
deterministic objective is used; if :math:`\lambda` is omitted, the original
deterministic constraint is used.  When both are provided they must share the
same distribution.  Deterministic bounds are attached with
:meth:`~otrobopt.RobustOptimizationProblem.setBounds`.  The class also rejects
a vector-valued robust objective; a reliability constraint may remain
vector-valued because its components are interpreted componentwise.

Discretization of measures
--------------------------

:class:`~otrobopt.MeasureFactory` converts a measure with a continuous
distribution into a measure with an empirical distribution,

.. math::

    \cD \;\longrightarrow\; \sum_{i=1}^{N} \omega_i\, \delta_{\theta_i},

where :math:`\{\theta_i\}` and :math:`\{\omega_i\}` are generated by a
:class:`~openturns.WeightedExperiment`, for example Monte Carlo, Latin
hypercube sampling or a Sobol sequence.  The discrete measure replaces the
integral by a finite sum, which makes the problem tractable with deterministic
solvers.

Sequential Monte Carlo algorithm
--------------------------------

:class:`~otrobopt.SequentialMonteCarloRobustAlgorithm` iteratively refines the
discretization and re-optimizes until convergence.  The algorithm is

.. math::

    \begin{array}{l}
    \textbf{Given: } N_0 \text{ (initial sample size)}, \;
      \varepsilon \text{ (convergence tolerance)} \\
    \textbf{Initialize: } N \leftarrow 0 \\
    \textbf{Repeat:} \\
    \qquad \Delta N \leftarrow \text{samplingSizeIncrement}(\max(N, N_0)) \\
    \qquad \text{Draw } \Delta N \text{ new samples }
      \{\theta_i\} \text{ from } \cD \\
    \qquad \text{Append to the existing sample; } N \leftarrow N + \Delta N \\
    \qquad \text{Discretize the measures with }
      \texttt{MeasureFactory} \\
    \qquad \text{Solve the deterministic problem from } x_{k-1} \\
    \qquad \text{Let } x_k \text{ be the new optimum} \\
    \qquad \text{Stop when } \|x_k - x_{k-1}\| < \varepsilon
    \end{array}

The sample starts empty: the first iteration draws
:math:`\text{samplingSizeIncrement}(N_0)` samples.  With the default identity
increment, this is :math:`N_0` samples, so the first deterministic problem is
solved with :math:`N_0` samples and not :math:`2N_0`.

The main aspects are the following.

* **Sample size.**  New samples are added at each iteration to improve the
  accuracy of the discretization.  The increment is configurable; the default
  adds :math:`N` new samples.
* **Multi-start.**  On the first iteration an optional multi-start search over
  the bounds, driven by a Sobol low-discrepancy sequence, provides a promising
  starting point.
* **Convergence.**  The algorithm stops when the change of the design point
  falls below the absolute error tolerance.
* **Solver tolerance.**  The tolerance of the inner deterministic solver is
  tightened as :math:`\varepsilon_k = C/\sqrt{N}`, where :math:`C` is a
  configurable factor.

The sequential approach avoids committing to a fixed discretization level
*before* the optimization: the approximation improves as the optimization
progresses, which yields efficient and robust convergence.

Measure summary
---------------

.. list-table:: Outputs and intended use of the measures.
   :header-rows: 1

   * - Measure
     - Output
     - Use
   * - :class:`~otrobopt.MeanMeasure`
     - scalar or vector
     - objective, risk-neutral
   * - :class:`~otrobopt.VarianceMeasure`
     - scalar or vector
     - objective, risk-averse
   * - :class:`~otrobopt.MeanStandardDeviationTradeoffMeasure`
     - scalar or vector
     - objective, risk-averse
   * - :class:`~otrobopt.QuantileMeasure`
     - scalar
     - objective, risk-averse
   * - :class:`~otrobopt.WorstCaseMeasure`
     - scalar or vector
     - objective, worst case
   * - :class:`~otrobopt.JointChanceMeasure`
     - scalar
     - constraint, joint reliability
   * - :class:`~otrobopt.IndividualChanceMeasure`
     - vector
     - constraint, componentwise reliability
   * - :class:`~otrobopt.AggregatedMeasure`
     - concatenation
     - assembly of several measures

The scalar-objective restriction of
:class:`~otrobopt.RobustOptimizationProblem` applies to the assembled
objective, not to the individual measure: the variance and the tradeoff
measure accept vector-valued functions, but their output must be scalar when
they define the robust objective.
