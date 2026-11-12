.. _robust-optimization-invariance:

Pushforward, invariance and canonicalization
============================================

.. currentmodule:: otrobopt

This page states the law-determined property, the pushforward identity, the
invariance theorem, the existence and measurability conditions, the
parameter-dependent construction and the discretization used by the
canonicalizer.

Law-determined measures
-----------------------

.. admonition:: Definition 1 (Law-determined measure)

   A measure functional :math:`\mathcal{M}` is *law-determined* if
   :math:`U \overset{d}{=} V` implies
   :math:`\mathcal{M}(U) = \mathcal{M}(V)` whenever both sides are defined.

The mean, the variance, the mean-standard-deviation tradeoff, the quantile,
the joint chance measure and the individual chance measure are law-determined
by construction: each is an integral, a root of a distribution function, or a
monotone transform of such a quantity.

The worst-case measure is not law-determined in general.  When the extremum is
taken over the support of the law, it is a function of the law.  When it is
taken over a fixed optimization domain, it is not: two variables with the same
law need not have the same extremum over that domain.  For example, a uniform
variable on :math:`[-1,1]` and its opposite have the same law, but their
minima over :math:`[0,1]` are :math:`0` and :math:`-1`.  The optimization
domain is therefore part of the model.  A canonicalization preserves a
worst-case extremum only when the domain is transported together with the law,
for example when it is the image of the reference domain under :math:`T_s`, or
when it is the support of :math:`\cD_s` itself.

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

   This is the standard change-of-measure identity for pushforward measures;
   see [Billingsley1995]_ and [Bogachev2007]_ for the measure-theoretic proof.

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

   The statement follows by applying Theorem 1 to the indicator of a Borel
   set and using Definition 1; see [Billingsley1995]_.

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
model; see :ref:`robust-optimization-parameter-dependence`.

.. _robust-optimization-parameter-dependence:

Parameter-dependent distributions and canonicalization
------------------------------------------------------

The parameter of the family is the *active* distribution parameter
:math:`s = \cD_s.\texttt{getParameter()}`.  The transformation :math:`T_s`
exposes the full parameter vector
:math:`T_s.\texttt{getFullParameter()}`, which also contains fixed or qualified
coordinates, typically the dependence or copula block.  The active indices are
available through :math:`T_s.\texttt{getActiveParameter()}`; the default
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
    T_s \circ \pi_{\xi,s}\bigr],

where :math:`\pi_x` selects the design coordinates and :math:`\pi_{\xi,s}`
selects the reference realization and the distribution parameter coordinates.
The result is wrapped in a :class:`~openturns.ParametricFunction` whose
parameter is the reference realization :math:`\xi`.  The measure then
integrates over the fixed :math:`\Xi`, while the optimizer sees
:math:`(x,s)`.

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

For an expectation measure, a sample :math:`\xi_1,\dots,\xi_N` from
:math:`\Xi` yields the sample-average problem

.. math::

    \min_{y \in \mathcal{Y}}\;
    \widehat{\mathcal{M}}_N(y)
    =
    \frac{1}{N}\sum_{i=1}^N h(y;\xi_i),

which is the empirical counterpart of
:math:`\Expect{h(y;\Xi)}`.  Other measures need their own empirical definition:
for example, the empirical variance also involves the empirical second moment,
and a chance constraint involves the empirical probability.  The convergence
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
