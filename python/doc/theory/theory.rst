.. _robust-optimization-theory:

Robust optimization theory
==========================

.. currentmodule:: otrobopt

This page states the mathematical contract of the robust optimization
features of the platform.  It covers the notation, the notion of a
law-determined measure, the pushforward identity, the canonicalization
theorem, the parameter-dependent construction, discretization, the specialized
algorithms and the validation benchmarks.  The measure catalogue and the API
mapping are in :doc:`robust_optimization`; the class reference is in the
:doc:`../user_manual/user_manual`.

.. contents:: Contents
   :local:
   :depth: 1

.. toctree::
   :hidden:

   robust_optimization

Notation and problem classes
----------------------------

Let :math:`x \in \Rset^{n_x}` be the design vector and let
:math:`\Theta \in \Rset^{n_\theta}` be the uncertain vector.  The
distribution of :math:`\Theta` belongs to a parametric family
:math:`\cD_s`, where the parameter :math:`s \in \Rset^{n_s}` collects the
parameters that may be optimized or frozen.  The objective is a measurable
map :math:`J : \Rset^{n_x} \times \Rset^{n_\theta} \to \Rset` and the
inequality constraint is a measurable map
:math:`G : \Rset^{n_x} \times \Rset^{n_\theta} \to \Rset^{n_g}`.

The original robust optimization problem reads

.. math::

    \begin{aligned}
    & \min_{x \in \mathcal{X}}
    & & \rho_{\cD_s}\bigl[J(x;\cdot)\bigr] \\
    & \text{subject to}
    & & \lambda_{\cD_s}\bigl[G(x;\cdot)\bigr] \ge 0,
    \end{aligned}

where :math:`\rho_{\cD_s}` is a robustness measure on the objective and
:math:`\lambda_{\cD_s}` is a reliability measure on the constraint.  Both are
law-determined functionals of the random vector :math:`J(x;\Theta)` or
:math:`G(x;\Theta)`.  The feasible design set :math:`\mathcal{X}` is described
by bounds, equality constraints and deterministic inequality constraints.

The canonical problem introduces a fixed reference distribution :math:`\Xi`
on :math:`\Rset^{n_\xi}` and a measurable transport
:math:`T_s : \Rset^{n_\xi} \to \Rset^{n_\theta}` such that the pushforward of
:math:`\Xi` by :math:`T_s` is :math:`\cD_s`.  It is

.. math::

    \begin{aligned}
    & \min_{(x,s) \in \mathcal{Y}}
    & & \rho_{\Xi}\bigl[F(x,s;\cdot)\bigr] \\
    & \text{subject to}
    & & \lambda_{\Xi}\bigl[G_F(x,s;\cdot)\bigr] \ge 0,
    \end{aligned}

with

.. math::

    F(x,s;\xi) = J\bigl(x;T_s(\xi)\bigr),
    \qquad
    G_F(x,s;\xi) = G\bigl(x;T_s(\xi)\bigr).

The decision vector of the canonical problem is :math:`y=(x,s)`.  When
:math:`s` is frozen, the decision vector is :math:`x` only; the fixed
distribution :math:`\cD_{s_0}` is then used directly as the reference.

It is essential to distinguish an *input* from a *parameter* in the OpenTURNS
sense.  The design vector :math:`x` is an input of every measure function.
The realization :math:`\theta` is a parameter of :math:`J` and :math:`G`: the
measure classes evaluate their function once per quadrature point or sample
point by setting that parameter.  In the canonical function, the reference
realization :math:`\xi` is the parameter, while :math:`(x,s)` are inputs.  The
dimensions are therefore

* :math:`\dim(\Xi) = n_\xi` for the measure distribution;
* :math:`\dim(\cD_s) = n_\theta` for the original distribution;
* :math:`n_x + n_s` inputs and :math:`n_\xi` parameters for the canonical
  function.

The following assumptions are maintained throughout.

* The distribution :math:`\cD_s` is continuous on :math:`\Rset^{n_\theta}`
  for every feasible :math:`s`; discrete distributions and mixtures with
  atoms are outside the scope of the first implementation.
* For every feasible :math:`s`, the transport :math:`T_s` is measurable and
  satisfies :math:`(T_s)_{\#}\Xi = \cD_s`.
* The objective :math:`J` is scalar-valued.  The constraint :math:`G` may be
  vector-valued.  A vector-valued robust objective is rejected by
  :class:`~otrobopt.RobustOptimizationProblem`.
* When both measures are present, they use the same parameter vector
  :math:`s` and the same family :math:`\cD_s`.  Independent blocks are
  aggregated first, as explained in `Parameter-dependent distributions and
  canonicalization`_.
* All moments, quantiles and probabilities that appear are finite, and the
  parameter qualifications (positive scales, positive degrees of freedom,
  positive definite correlation matrices) are enforced by the user bounds.

Law-determined measures
-----------------------

.. admonition:: Definition 1 (Law-determined measure)

   A measure functional :math:`\mathcal{M}` is *law-determined* if
   :math:`U \overset{d}{=} V` implies
   :math:`\mathcal{M}(U) = \mathcal{M}(V)` whenever both sides are defined.

The mean, the variance, the mean-standard-deviation tradeoff, the quantile,
the joint chance measure and the individual chance measure are law-determined
by construction: each is an integral, a root of a distribution function, or a
monotone transform of such a quantity.  The worst-case measure is
law-determined once the support of the law and the optimization domain are
fixed, because it is the extremum of a continuous function over that set.

The property is not automatic.  A pathwise maximum over a fixed sample is not
law-determined: it depends on the sample, not only on the law.  This is the
reason the worst-case measure needs an explicit support and domain policy, and
the reason a sample-average approximation is a numerical method rather than a
measure.

Pushforward and canonicalization
--------------------------------

.. admonition:: Theorem 1 (Pushforward identity)

   Let :math:`\Xi` be a probability measure on :math:`\Rset^{n_\xi}`, let
   :math:`T : \Rset^{n_\xi} \to \Rset^{n_\theta}` be measurable, and let
   :math:`\cD = T_{\#} \Xi`.  For every measurable
   :math:`h : \Rset^{n_x} \times \Rset^{n_\theta} \to \Rset^q` such that
   either side is integrable,

   .. math::

       \int_{\Rset^{n_\theta}} h(x,\theta)\,\cD(\mathrm{d}\theta)
       =
       \int_{\Rset^{n_\xi}} h\bigl(x,T(\xi)\bigr)\,\Xi(\mathrm{d}\xi)
       \qquad \text{for every } x.

   **Proof.**  For a product-measurable indicator
   :math:`h(x,\theta) = \mathbf{1}_A(x)\mathbf{1}_B(\theta)`, the left-hand
   side is :math:`\mathbf{1}_A(x)\cD(B) = \mathbf{1}_A(x)\Xi(T^{-1}(B))`,
   which is the right-hand side.  The identity extends by linearity to simple
   functions, by monotone convergence to non-negative measurable functions,
   and by taking positive and negative parts.  Applying the argument to the
   section :math:`h_x` gives the statement for fixed :math:`x`.  :math:`\square`

If :math:`\cD_s` and :math:`\Xi` have densities :math:`p_s` and
:math:`p_\Xi`, and :math:`T_s` is a diffeomorphism, the same identity is the
familiar change-of-variables formula

.. math::

    p_s\bigl(T_s(\xi)\bigr)\,
    \bigl|\det D T_s(\xi)\bigr| = p_\Xi(\xi).

The measure-theoretic form is preferable here because isoprobabilistic
transformations can be non-smooth at the boundary of a support, for example a
quantile transformation from a normal reference to a bounded uniform
distribution.

.. admonition:: Theorem 2 (Invariance)

   Let :math:`\Theta(s) \sim \cD_s` and let :math:`\Xi` and :math:`T_s`
   satisfy :math:`(T_s)_{\#}\Xi = \cD_s`.  Then, for every :math:`x`
   and :math:`s`,

   .. math::

       J\bigl(x;\Theta(s)\bigr) \overset{d}{=}
       J\bigl(x;T_s(\Xi)\bigr),
       \qquad
       G\bigl(x;\Theta(s)\bigr) \overset{d}{=}
       G\bigl(x;T_s(\Xi)\bigr).

   Consequently, for every law-determined measure :math:`\mathcal{M}`,

   .. math::

       \mathcal{M}_{\cD_s}\bigl[J(x;\cdot)\bigr]
       =
       \mathcal{M}_{\Xi}\bigl[F(x,s;\cdot)\bigr],

   with :math:`F(x,s;\xi) = J(x;T_s(\xi))`, and similarly for :math:`G` and
   the reliability measure.

   **Proof.**  For a Borel set :math:`B \subseteq \Rset`,

   .. math::

       \Prob{J(x;\Theta(s)) \in B}
       =
       \cD_s\bigl(\{\theta : J(x;\theta) \in B\}\bigr)
       =
       \Xi\bigl(\{\xi : J(x;T_s(\xi)) \in B\}\bigr),

   where the last equality is the pushforward identity applied to the
   indicator of :math:`\{\theta : J(x;\theta) \in B\}`.  Hence the two scalar
   random variables have the same law.  The vector case is identical,
   component by component.  The measure equality follows from
   Definition 1.  :math:`\square`

The theorem does not require :math:`T_s` to be invertible.  It only requires
the pushforward equality.  Invertibility is needed for round-trip diagnostics
and for the support statement of the worst-case measure, not for the law of
:math:`J(x;\Theta(s))`.

The reference :math:`\Xi` must not depend on :math:`s`.  If it did, the
canonical measure would be :math:`\mathcal{M}_{\Xi(s)}`, and the measure
distribution would have to be rebuilt at every evaluation of the optimizer.
The measure protocol of :class:`~otrobopt.MeasureEvaluation` owns one
distribution and the function owns one parameter vector; a fixed reference
keeps the parameter dimension, the quadrature grid and the common random
numbers fixed.  The price is that :math:`T_s` must be rebuilt, or at least
re-parameterized, when :math:`s` changes.  The class
:class:`~otrobopt.ParametricDistributionTransformation` does exactly that.

Existence and joint measurability
---------------------------------

For a continuous distribution :math:`\cD_s`, a transport :math:`T_s` exists.
One construction is the Rosenblatt transform composed with its inverse;
another is the quantile transform in dimension one; the
:class:`~openturns.DistributionTransformation` class provides the
isoprobabilistic constructions.  The transport need not be unique and need
not be invertible: only the pushforward equality matters.

When :math:`s` is a decision variable, the map
:math:`(s,\xi) \mapsto T_s(\xi)` must be jointly measurable and
:math:`J` must be a Caratheodory function, so that
:math:`F(x,s;\xi) = J(x;T_s(\xi))` is a well-defined function of
:math:`y=(x,s)`.  The isoprobabilistic constructions are expected to satisfy
this condition on the feasible parameter domain, except at singular values
such as a zero scale or a singular correlation matrix.  Those values are
excluded by the parameter bounds.  Joint measurability is not needed for the
pointwise theorem; it is needed when :math:`s` is optimized, and it is stated
here as an assumption rather than claimed without proof.

The implementation uses two reference policies.  The *standard fast path*
applies when :math:`\cD_s` has a parameter-free standard distribution
:math:`\Xi_0`, for example a normal with fixed standard parameters.  The
*normal fallback* uses :math:`\Xi = \mathcal{N}(m)` with
:math:`m = \dim(\cD_s)` when the fast path is not available.  A worst-case
measure needs an additional domain policy because the support is part of the
model; see `Parameter-dependent distributions and canonicalization`_.

Measure catalogue
-----------------

The measures exposed by otrobopt are summarized below.  The detailed
definitions, the output dimensions and the class links are in
:doc:`robust_optimization`.

.. list-table::
   :header-rows: 1

   * - Measure
     - Output
     - Definition
   * - :class:`~otrobopt.MeanMeasure`
     - scalar
     - :math:`\Expect{J(x;\Theta)}`
   * - :class:`~otrobopt.VarianceMeasure`
     - scalar
     - :math:`\Var{J(x;\Theta)}`
   * - :class:`~otrobopt.MeanStandardDeviationTradeoffMeasure`
     - scalar
     - :math:`(1-\alpha)\Expect{J} + \alpha\sqrt{\Var{J}}`
   * - :class:`~otrobopt.QuantileMeasure`
     - scalar
     - :math:`\inf\{t : \Prob{J(x;\Theta) \le t} \ge \alpha\}`
   * - :class:`~otrobopt.JointChanceMeasure`
     - scalar
     - probit transform of :math:`\Prob{G(x;\Theta) \ge 0}`
   * - :class:`~otrobopt.IndividualChanceMeasure`
     - vector
     - componentwise probit transform of :math:`\Prob{G_k(x;\Theta) \ge 0}`
   * - :class:`~otrobopt.WorstCaseMeasure`
     - scalar
     - extremum over the support or the optimization domain
   * - :class:`~otrobopt.AggregatedMeasure`
     - concatenation
     - concatenation of several measure outputs

The chance measures use the event :math:`G(x;\Theta) \ge 0` for every
component.  The comparison operator selects the returned value:

.. math::

    \lambda =
    \begin{cases}
      \Phi^{-1}\bigl(\Prob{G(x;\Theta) \ge 0}\bigr) - \Phi^{-1}(\alpha),
      & \text{with \texttt{GreaterOrEqual}},\\
      \Phi^{-1}(\alpha) - \Phi^{-1}\bigl(\Prob{G(x;\Theta) \ge 0}\bigr),
      & \text{with \texttt{Less}}.
    \end{cases}

For a reliability constraint :math:`\Prob{G(x;\Theta) \ge 0} \ge \alpha`, use
:class:`~openturns.GreaterOrEqual`.  The ``Less`` form is the complement form:
it represents :math:`\Prob{G(x;\Theta) \ge 0} \le \alpha`, equivalently
:math:`\Prob{G(x;\Theta) < 0} \ge 1-\alpha`.  The operator does not change the
event that is integrated; it only changes the sign of the returned measure.
This distinction is important when a chance constraint is rebuilt.

For :class:`~otrobopt.IndividualChanceMeasure`, the output is a vector with
one entry per constraint component and the level :math:`\alpha` is a vector
with the same dimension.  The constraint
:math:`\lambda(x) \ge 0` is interpreted componentwise.

Parameter-dependent distributions and canonicalization
------------------------------------------------------

The parameter of the family is the *active* distribution parameter
:math:`s = \cD_s.\texttt{getParameter()}`.  The full parameter vector
:math:`\cD_s.\texttt{getFullParameter()}` also contains fixed or qualified
coordinates, typically the dependence or copula block.  The active indices are
available through :math:`\cD_s.\texttt{getActiveParameter()}`; the default
policy keeps every parameter active except the trailing dependence block
exposed by ``Distribution.getParametersCollection``, because OpenTURNS does
not provide an analytic parameter derivative for that block.  The user can
override the selection with ``setActiveParameter`` on the transformation or
with ``CanonicalizationOptions.activeParameter`` on the canonicalizer.

The canonical decision vector is :math:`y=(x,s)`.  The canonicalizer
:class:`~otrobopt.RobustOptimizationProblemCanonicalizer` builds the canonical
function

.. math::

    F(x,s;\xi) = J\bigl(x;T_s(\xi)\bigr)

and rebuilds every measure with the fixed reference :math:`\Xi`.  The active
distribution parameter is exposed as an input of the canonical objective and
of the canonical constraints.  The class
:class:`~otrobopt.CanonicalizationOptions` controls the reference, the active
parameter, the parameter bounds, the parameter starting point and the
worst-case domain.  Bounds and starting points are expressed in active
coordinates; a full-dimensional value is accepted as a compatibility fallback
and is filtered to the active coordinates.

The reference selection follows these rules.

* ``AUTO`` uses an explicit reference when one is set, then the standard fast
  path when it is available, and then :math:`\mathcal{N}(n_\theta)`.
* ``STANDARD`` requires the standard fast path and rejects a user reference.
* ``NORMAL`` forces :math:`\mathcal{N}(n_\theta)` and rejects a user
  reference.
* ``USER_REFERENCE`` requires an explicit reference.
* ``USER_DOMAIN`` uses the automatic reference policy and requires a
  worst-case domain.

When ``optimizeParameter`` is false, or when the active parameter is empty,
the parameter is frozen and the distribution :math:`\cD_{s_0}` is used
directly as the reference.  The canonical decision vector is :math:`x` only
and the canonicalization is exact without inventing a worst-case domain.  When
it is true, a worst-case measure fails closed unless the standard fast path, a
bounded user reference or an explicit domain is available.  The policies
``STANDARD``, ``NORMAL``, ``USER_REFERENCE`` and ``USER_DOMAIN`` make the
choice explicit.

The canonical function is assembled from the original function and the
transport.  For a measure function :math:`J`, the composition is

.. math::

    H = J_{\mathrm{in}} \circ
    \operatorname{AggregatedFunction}\bigl[\pi_x,\,
    T_s \circ \pi_{x,s}\bigr],

where :math:`\pi_x` selects the design coordinates and :math:`\pi_{x,s}`
selects the distribution parameter coordinates.  The result is wrapped in a
:class:`~openturns.ParametricFunction` whose parameter is the reference
realization :math:`\xi`.  The measure then integrates over the fixed
:math:`\Xi`, while the optimizer sees :math:`(x,s)`.

The :class:`~otrobopt.AggregatedMeasure` is canonicalized child by child.
The aggregate function concatenates the child parameter vectors, so
canonicalizing the aggregate as a single function would corrupt the
parameter order.  Each child is rebuilt with its own function and the shared
distribution, and the collection is reassembled.

The robust problem rejects mixed measure distributions.  If the objective
measure and the reliability measure use different distributions, the
canonicalizer cannot choose one parameter vector :math:`s`; the error points
to the aggregation construction.  The same check is applied recursively
inside an :class:`~otrobopt.AggregatedMeasure`.  A vector-valued robust
objective is rejected as well.

When the model is assembled from independent blocks, the supported
construction is user-side.  Let

.. math::

    \cD_s = \operatorname{BID}(\cD_{1,s_1}, \dots, \cD_{K,s_K})

be a :class:`~openturns.BlockIndependentDistribution`.  Its parameter is the
concatenation :math:`s=(s_1,\dots,s_K)`, and its isoprobabilistic
transformation acts blockwise.  For each block, define the selector
:math:`A_k(\theta) = \theta_k` as a
:class:`~openturns.SymbolicFunction`; compose the block-dependent function
with :math:`A_k` and freeze the aggregate realization as the parameter.  The
result uses one distribution and one parameter vector, so the canonicalizer
applies without a special case.  The limitation is that the blocks must be
independent; dependent blocks must be described by a single joint
distribution.

The main failure modes are:

* a stale or wrong transport, so that
  :math:`(T_s)_{\#}\Xi \ne \cD_s`;
* a reference that still depends on :math:`s`, for example a standard
  distribution whose degrees of freedom are not fixed;
* a support mismatch for the worst-case measure, where the mathematical
  extremum is preserved but the numerical domain is not;
* a discrete or mixed distribution, which is outside the continuous
  transport assumption;
* unaggregated parameter blocks, which the canonicalizer rejects;
* a vector-valued robust objective, which the problem rejects;
* a measure that was discretized before canonicalization, so that the
  distribution parameter is no longer available;
* a shallow copy of an aggregate measure, which does not rebuild its
  children;
* a sign flip in a chance constraint, which turns the constraint into its
  complement;
* numerical equality assumptions: the theorem is exact, but quadrature,
  sampling and finite differences are not.

Discretization and common random numbers
----------------------------------------

:class:`~otrobopt.MeasureFactory` replaces a continuous measure by a measure
with an empirical distribution,

.. math::

    \cD \;\longrightarrow\; \sum_{i=1}^N \omega_i\,\delta_{\xi_i},

where :math:`\{\xi_i\}` and :math:`\{\omega_i\}` are generated by a
:class:`~openturns.WeightedExperiment`, for example Monte Carlo, Latin
hypercube sampling or a Sobol sequence.  The discrete measure replaces the
integral by a finite sum and makes the problem tractable with deterministic
solvers.

For a sample :math:`\xi_1,\dots,\xi_N` from :math:`\Xi`, the sample-average
problem is

.. math::

    \min_{y \in \mathcal{Y}}\;
    \widehat{\mathcal{M}}_N(y)
    =
    \frac{1}{N}\sum_{i=1}^N h(y;\xi_i),

with the obvious extension to chance constraints.  The convergence
assumptions are integrability, compactness of :math:`\mathcal{Y}` and
continuity of :math:`h`; chance constraints require a separate probabilistic
guarantee because the empirical probability is a step function.  Common
random numbers, that is the same sample for the objective and the
constraints, reduce the variance but do not remove the bias of a fixed
sample.

The canonical problem makes common random numbers automatic: the fixed
reference :math:`\Xi` and the single
:class:`~otrobopt.MeasureFactory` discretization provide the shared sample.
This is also the reason :class:`~otrobopt.SequentialMonteCarloRobustAlgorithm`
can refine the sample while keeping a single deterministic problem per
iteration.

Specialized algorithms
----------------------

The generic :class:`~otrobopt.RobustOptimizationProblem` accepts any robustness
or reliability measure, and the canonicalizer reduces it to a deterministic
problem over :math:`(x,s)`.  Some algorithms exploit a narrower structure and
therefore use the dedicated :class:`~otrobopt.RBDOProblem`:

* a scalar classical :class:`~openturns.Function` objective;
* one :class:`~otrobopt.JointChanceMeasure` constraint;
* deterministic bounds, inequality constraints and equality constraints.

The class keeps the distribution parameter fixed by default.  The canonical
problem returned by :meth:`~otrobopt.RBDOProblem.getCanonicalProblem` freezes
it; the overloads that accept
:class:`~otrobopt.CanonicalizationOptions` expose the active parameter
:math:`s` in the canonical decision vector.  The canonical inequality
constraint gathers the joint chance index first and the deterministic
inequalities afterwards.  The measure itself stays available through
:meth:`~otrobopt.RBDOProblem.getCanonicalJointChanceMeasure`, so an algorithm
reads the target probability and the comparison operator from the measure
instead of inferring them from the index value.

The literature review of RBDO led to the following design choices.  The
double-loop formulation with RIA or PMA is the reference implementation
[Aoues2010]_, [Tu1999]_, [Valdebenito2010]_.  SORA decouples the loops through
a shift of the deterministic constraints [Du2004]_, and the single-loop
methods replace the inner reliability problem by a fixed-point update
[Liang2004]_.  The sample-average and scenario approximations are
distribution-free references [Shapiro2009]_, [Calafiore2006]_,
[Rocchetta2021]_, while joint chance constraints over finite distributions
admit exact mixed-integer formulations [Luedtke2010]_, [Ahmed2018]_.
Analytic approximations and gradients for joint chance constraints are
available for specific families [Geletu2019]_, [Xie2019]_,
[VanAckooij2014]_, and sequential convex approaches require additional
structure [Hong2011]_.

Most existing tools implement component RBDO rather than a single joint chance
event: DAKOTA provides RIA and PMA and a bi-level workflow [Eldred2007]_,
UQpy and Fortuna.jl provide reliability primitives [Olivier2020]_,
[Akchurin2024]_, FERUM is a research code [DerKiureghian2006]_, and the R
ecosystem supplies building blocks [Mullen2011]_.  This is why the extension
makes the joint-chance treatment explicit and keeps a documented Bonferroni
fallback instead of switching silently between estimators.

The algorithms are added incrementally: the double-loop RIA/PMA baseline and
the sample-average reference come first, then SORA with an explicit
joint-chance mode.  Single-loop, scenario/cutting-plane and bilevel worst-case
methods follow in a second phase, and the alternating parameter heuristic in
a third phase.  The applicability conditions are the continuity of the limit
state, the availability of a standard space for the reliability index, and the
convexity or smoothness assumptions required by the deterministic solver.
Problems outside the documented class are rejected with an explicit message.

Worked examples and validation
------------------------------

Running example
^^^^^^^^^^^^^^^

Take :math:`J(x;\theta) = (x-\theta)^2` and
:math:`\cD_s = \mathcal{N}(\mu,\sigma^2)` with :math:`s=(\mu,\sigma)`.  Then

.. math::

    \mathcal{M}_{\cD_s}\bigl[J(x;\cdot)\bigr]
    =
    \Expect{(x-\Theta)^2}
    =
    (x-\mu)^2 + \sigma^2.

With :math:`\Xi = \mathcal{N}(0,1)` and :math:`T_s(\xi)=\mu+\sigma\xi`,

.. math::

    F(x,\mu,\sigma;\xi) = (x-\mu-\sigma\xi)^2,

and :math:`\Econd{\Xi}{F} = (x-\mu)^2+\sigma^2`.  The canonical problem
optimizes over :math:`(x,\mu,\sigma)` while integrating over the fixed
standard normal.  The variance is

.. math::

    \Var{J(x;\Theta)}
    =
    4\sigma^2(x-\mu)^2 + 2\sigma^4.

Normal mean and variance
^^^^^^^^^^^^^^^^^^^^^^^^

The validation test compares the canonical measure with a freshly
parameterized distribution for the normal, uniform, gamma, beta, lognormal,
exponential, Gumbel, logistic, Weibull minimum and triangular families.  The
mean and the variance agree to :math:`10^{-6}` and the :math:`0.9` quantile
agrees to :math:`2\times 10^{-2}`.  A correlated bivariate normal is checked
for the mean and the variance to :math:`10^{-6}`.

Uniform worst case
^^^^^^^^^^^^^^^^^^

For :math:`J(x;\theta)=x\theta` and
:math:`\Theta \sim \mathcal{U}(-1,2)`, the worst case over the support is

.. math::

    \min_{\theta \in [-1,2]} x\theta =
    \begin{cases}
      -x, & x \ge 0,\\
      -2x, & x < 0.
    \end{cases}

At :math:`x=1` the value is :math:`-1`, and at :math:`x=-1` it is :math:`-2`.
The canonical worst-case measure reproduces these values to
:math:`10^{-4}` through the standard fast path.

Student scale
^^^^^^^^^^^^^

For a one-dimensional Student distribution with scale parameter
:math:`\sigma`, the generic path is expected to fail until the OpenTURNS
defects tracked by issues #3418--#3424 and PR #3422 are fixed.  The mean of a
linear function is preserved, but the validation records:

* variance: canonical :math:`9` versus fresh :math:`12`;
* :math:`0.9` quantile: canonical :math:`5.34465` versus fresh
  :math:`5.69045`;
* joint chance: canonical :math:`-0.78155` versus fresh :math:`-1.28155`;
* bivariate elliptical variance: canonical :math:`8.38849` versus fresh
  :math:`7.73333`.

These checks are explicit xfails in the test suite.  The test fails if one of
them starts passing, so that the xfails are removed when the OpenTURNS fixes
are available.

Chance constraint with a linear limit state
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Minimize :math:`\Expect{x-\Theta}=x-\mu` subject to
:math:`\Prob{x-\Theta \ge 0} \ge 0.9` with
:math:`\Theta \sim \mathcal{N}(\mu,\sigma^2)`, :math:`x \in [0,5]`,
:math:`\mu \in [1,2]` and :math:`\sigma \in [0.1,0.5]`.  The constraint is

.. math::

    \frac{x-\mu}{\sigma} \ge z_{0.9},
    \qquad z_{0.9} = \Phi^{-1}(0.9) \approx 1.28155.

The optimal value is :math:`\sigma z_{0.9}`.  The deterministic solver
returns :math:`\sigma^\star = 0.1`,
:math:`x^\star = \mu^\star + 0.1 z_{0.9} \approx 2.12816` and the optimal
value :math:`0.12816`.  The parameter :math:`\mu` is not unique because the
reduced objective depends only on :math:`\sigma`; the test checks the
invariant :math:`x^\star = \mu^\star + \sigma^\star z_{0.9}`.  With the lower
bound :math:`\sigma \ge 0.2`, the optimal value becomes
:math:`0.2 z_{0.9} \approx 0.25631`.  The frozen-parameter canonical problem
reproduces the direct solution of the original problem to :math:`10^{-5}`.

The validation is implemented in
``python/test/t_RobustOptimizationProblemCanonicalizer_validation_std.py``
and is registered with ``IGNOREOUT``.  The cached transformation is about four
times faster than rebuilding it at each evaluation in the validation run.

Bibliographic notes
-------------------

The classical robust optimization background is in [BenTal2009]_,
[Beyer2007]_ and [Bertsimas2004]_.  Reliability-based design and its
benchmarks are covered by [Aoues2010]_, [Valdebenito2010]_, [Du2004]_,
[Tu1999]_, [Liang2004]_, [Lemaire2009]_, [Ditlevsen1996]_, [Hasofer1974]_
and [Rackwitz1978]_.  Chance constraints and stochastic programming are
treated in [Prekopa1995]_, [Shapiro2003]_, [Shapiro2009]_,
[Nemirovski2006]_, [Chen2010]_, [Rockafellar2000]_ and [Calafiore2006]_.
The isoprobabilistic constructions are due to [Rosenblatt1952]_,
[Nataf1962]_, [Hohenbichler1981]_, [Lebrun2009a]_, [Lebrun2009b]_ and
[Lebrun2009c]_.  Measure-theoretic and Monte Carlo foundations are in
[Billingsley1995]_, [Bogachev2007]_ and [Glasserman2003]_.  The
reparameterization identity used by stochastic optimization is discussed by
[Kingma2014]_ and [Rezende2014]_.

References
----------

.. [Aoues2010] A. Aoues and D. Chateauneuf. Benchmark study of numerical
   methods for reliability-based design optimization. *Structural and
   Multidisciplinary Optimization*, 41(2):277--294, 2010.

.. [BenTal2009] A. Ben-Tal, L. El Ghaoui and A. Nemirovski.
   *Robust Optimization*. Princeton University Press, 2009.


.. [Bogachev2007] V. I. Bogachev. *Measure Theory*. Springer, 2007.

.. [Chen2010] W. Chen, M. Sim, J. Sun and C.-P. Teo. From CVaR to uncertainty
   set: implications in joint chance-constrained optimization. *Operations
   Research*, 58(2):470--485, 2010.

.. [Ditlevsen1996] O. Ditlevsen and H. O. Madsen. *Structural Reliability
   Methods*. Wiley, 1996.

.. [Du2004] X. Du and W. Chen. Sequential optimization and reliability
   assessment method for probabilistic optimization. *Structural and
   Multidisciplinary Optimization*, 27(2):120--130, 2004.

.. [Glasserman2003] P. Glasserman. *Monte Carlo Methods in Financial
   Engineering*. Springer, 2003.

.. [Hasofer1974] A. M. Hasofer and N. C. Lind. Exact and invariant second
   moment code format. *Journal of the Engineering Mechanics Division*,
   100(1):111--121, 1974.

.. [Hohenbichler1981] M. Hohenbichler and R. Rackwitz. Non-normal dependent
   vectors in structural reliability. *Journal of Engineering Mechanics*,
   107(6):1227--1238, 1981.

.. [Kingma2014] D. P. Kingma and M. Welling. Auto-encoding variational Bayes.
   In *International Conference on Learning Representations*, 2014.

.. [Lebrun2009a] R. Lebrun and A. Dutfoy. An innovating analysis of the Nataf
   transformation from the copula viewpoint. *Probabilistic Engineering
   Mechanics*, 24(3):312--320, 2009.

.. [Lebrun2009b] R. Lebrun and A. Dutfoy. A generalization of the Nataf
   transformation to distributions with elliptical copula. *Probabilistic
   Engineering Mechanics*, 24(2):172--178, 2009.

.. [Lebrun2009c] R. Lebrun and A. Dutfoy. Do Rosenblatt and Nataf
   isoprobabilistic transformations really differ? *Probabilistic Engineering
   Mechanics*, 24(4):577--584, 2009.

.. [Lemaire2009] M. Lemaire, A. Chateauneuf and J.-C. Mitteau. *Structural
   Reliability and Optimal Design*. Springer, 2009.

.. [Liang2004] J. Liang, Z. P. Mourelatos and J. Tu. A single-loop method for
   reliability-based design optimization. In *Proceedings of the ASME Design
   Engineering Technical Conferences*, 2004.

.. [Nataf1962] A. Nataf. Determination des distributions de probabilites dont
   les marges sont donnees. *Comptes Rendus de l'Academie des Sciences*,
   255:42--43, 1962.

.. [Nemirovski2006] A. Nemirovski and A. Shapiro. Convex approximations of
   chance constrained programs. *SIAM Journal on Optimization*,
   17(4):969--996, 2006.

.. [Prekopa1995] A. Prekopa. *Stochastic Programming*. Kluwer, 1995.

.. [Rackwitz1978] R. Rackwitz and B. Fiessler. Structural reliability under
   combined random load sequences. *Computers & Structures*, 9(5):489--494,
   1978.

.. [Rezende2014] D. J. Rezende, S. Mohamed and D. Wierstra. Stochastic
   backpropagation and approximate inference in deep latent Gaussian models.
   In *International Conference on Machine Learning*, 2014.

.. [Rockafellar2000] R. T. Rockafellar and S. Uryasev. Optimization of
   conditional value-at-risk. *Journal of Risk*, 2(3):21--41, 2000.

.. [Rosenblatt1952] M. Rosenblatt. Remarks on a multivariate transformation.
   *Annals of Mathematical Statistics*, 23(3):470--472, 1952.

.. [Shapiro2003] A. Shapiro. Monte Carlo sampling methods. In A. Ruszczynski
   and A. Shapiro, editors, *Stochastic Programming*, Handbooks in Operations
   Research and Management Science, pages 353--425. Elsevier, 2003.

.. [Shapiro2009] A. Shapiro, D. Dentcheva and A. Ruszczynski. *Lectures on
   Stochastic Programming*. SIAM, 2009.

.. [Tu1999] J. Tu, K. K. Choi and Y. H. Park. A new study on reliability-based
   design optimization. *Journal of Mechanical Design*, 121(4):557--564,
   1999.

.. [Valdebenito2010] M. A. Valdebenito and G. I. Schueller. A survey on
   approaches for reliability-based optimization. *Structural and
   Multidisciplinary Optimization*, 42(1):1--24, 2010.

.. [Beyer2007] H.-G. Beyer and B. Sendhoff. Robust optimization -- a
   comprehensive survey. *Computer Methods in Applied Mechanics and
   Engineering*, 196(33--34):3190--3218, 2007.

.. [Bertsimas2004] D. Bertsimas and M. Sim. The price of robustness.
   *Operations Research*, 52(1):35--53, 2004.

.. [Calafiore2006] G. C. Calafiore and M. C. Campi. The scenario approach to
   robust control design. *IEEE Transactions on Automatic Control*,
   51(5):742--752, 2006.

.. [Billingsley1995] P. Billingsley. *Probability and Measure*. Wiley, 1995.

.. [Ahmed2018] S. Ahmed and W. Xie. Relaxations and approximations of chance
   constraints under finite distributions. *Mathematical Programming*,
   170(1):43--65, 2018.

.. [Akchurin2024] D. Akchurin. Fortuna.jl: structural and system reliability
   analysis in Julia. *Journal of Open Source Software*, 9(100):6967, 2024.

.. [DerKiureghian2006] A. Der Kiureghian, T. Haukaas and K. Fujimura.
   Structural reliability software at the University of California, Berkeley.
   *Structural Safety*, 28(1--2):44--67, 2006.

.. [Eldred2007] M. S. Eldred, H. Agarwal, V. M. Perez, S. F. Wojtkiewicz and
   J. E. Renaud. Investigation of reliability method formulations in
   DAKOTA/UQ. *Structure and Infrastructure Engineering*, 3(3):199--213, 2007.

.. [Geletu2019] A. Geletu, A. Hoffmann and P. Li. Analytic approximation and
   differentiability of joint chance constraints. *Optimization*,
   68(10):1985--2023, 2019.

.. [Hong2011] L. J. Hong, Y. Yang and L. Zhang. Sequential convex
   approximations to joint chance constrained programs: a Monte Carlo
   approach. *Operations Research*, 59(3):617--630, 2011.

.. [Luedtke2010] J. Luedtke, S. Ahmed and G. Nemhauser. An integer programming
   approach for linear programs with probabilistic constraints.
   *Mathematical Programming*, 122(2):247--272, 2010.

.. [Mullen2011] K. Mullen, D. Ardia, D. Gil, D. Windover and J. Cline.
   DEoptim: an R package for global optimization by differential evolution.
   *Journal of Statistical Software*, 40(6):1--26, 2011.

.. [Olivier2020] A. Olivier, D. G. Giovanis, B. S. Aakash, M. Chauhan,
   L. Vandanapu and M. D. Shields. UQpy: a general purpose Python package and
   development environment for uncertainty quantification. *Journal of
   Computational Science*, 47:101204, 2020.

.. [Rocchetta2021] R. Rocchetta and L. G. Crespo. A scenario optimization
   approach to reliability-based and risk-based design: soft-constrained
   modulation of failure probability bounds. *Reliability Engineering and
   System Safety*, 216:107900, 2021.

.. [VanAckooij2014] W. van Ackooij and R. Henrion. Gradient formulae for
   nonlinear probabilistic constraints with Gaussian and Gaussian-like
   distributions. *SIAM Journal on Optimization*, 24(4):1864--1889, 2014.

.. [Xie2019] W. Xie, S. Ahmed and R. Jiang. Optimized Bonferroni
   approximations of distributionally robust joint chance constraints.
   *Mathematical Programming*, 191(1):79--112, 2022.
