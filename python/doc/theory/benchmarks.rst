.. _robust-optimization-benchmarks:

Worked examples and validation
==============================

.. currentmodule:: otrobopt

Running example
---------------

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
------------------------

The validation test compares the canonical measure with a freshly
parameterized distribution for the normal, uniform, gamma, beta, lognormal,
exponential, Gumbel, logistic, Weibull minimum and triangular families.  The
mean and the variance agree to :math:`10^{-6}` and the :math:`0.9` quantile
agrees to :math:`2\times 10^{-2}`.  A correlated bivariate normal is checked
for the mean and the variance to :math:`10^{-6}`.

Uniform worst case
------------------

For :math:`J(x;\theta)=x\theta` and
:math:`\Theta \sim \mathcal{U}(-1,2)`, the worst case over the support is

.. math::

    \min_{\theta \in [-1,2]} x\theta =
    \begin{cases}
      -x, & x \ge 0,\\
      2x, & x < 0.
    \end{cases}

At :math:`x=1` the value is :math:`-1`, and at :math:`x=-1` it is :math:`-2`.
The canonical worst-case measure reproduces these values to
:math:`10^{-4}` through the standard fast path.

Student scale
-------------

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
-------------------------------------------

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

Two closed-form benchmarks
--------------------------

Two further problems have a closed-form solution and are used to validate the
parameter-dependent construction end to end.  They are implemented in
``python/test/t_RobustOptimizationProblemCanonicalizer_benchmarks_std.py`` and
are also the two gallery examples ``plot_canonical_correlated_normal.py`` and
``plot_canonical_uniform_bound.py``, so the figures and the test exercise the
same construction.

Correlated normal with an expected-separation constraint
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Let :math:`\Theta \sim \mathcal{N}(\mu,\Sigma)` with fixed means
:math:`\mu=(2,2)`, marginal standard deviations :math:`\sigma_1,\sigma_2` and
correlation :math:`\rho`.  The decision vector is
:math:`(x_1,x_2,\sigma_1,\sigma_2,\rho)`, and the problem is

.. math::

    \begin{aligned}
    \min_{x,\sigma,\rho} \quad
    & \Expect{10(\Theta_1-x_1)^2 + (\Theta_2-x_2)^2} \\
    \text{subject to} \quad
    & \Expect{(\Theta_1-\Theta_2)^2} \ge 0.6,
    \end{aligned}

with :math:`x \in [0,4]^2`, :math:`\sigma_i \in [0.2,1.5]` and
:math:`\rho \in [-0.8,0.8]`.  OpenTURNS orders the normal parameter as
:math:`(\mu_0,\sigma_0,\mu_1,\sigma_1,R_{1,0})`, so the active parameter is
``[1, 3, 4]`` and the canonical problem has dimension five.

The separation constraint reduces to

.. math::

    \sigma_1^2 + \sigma_2^2 - 2\rho\sigma_1\sigma_2 = 0.6,
    \qquad\text{that is}\qquad
    \sigma_2 = \rho\sigma_1 +
    \sqrt{0.6 - (1-\rho^2)\sigma_1^2}.

At the optimum the constraint is active, the correlation and the expensive
standard deviation sit at their lower bounds,
:math:`\sigma_1=0.2` and :math:`\rho=-0.8`, and

.. math::

    \sigma_2 = -0.8(0.2) + \sqrt{0.6 - (1-0.8^2)(0.2)^2}
    \approx 0.6052.

The optimal objective is
:math:`10(0.2)^2 + (0.6052)^2 \approx 0.7663`, and the optimal design is
:math:`x=(2,2)`.  The canonical measures agree with a freshly parameterized
distribution, and Cobyla recovers the analytic point from the canonical
starting point in twenty iterations.  The test checks the optimum to
:math:`10^{-3}` and the analytic objective and constraint to
:math:`10^{-4}`.

Uniform distribution with a moving support
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Let :math:`\Theta \sim \mathcal{U}(0,b)` and let :math:`b` be a decision
variable.  The problem is

.. math::

    \begin{aligned}
    \min_{x,b} \quad
    & \Expect{(\Theta-x)^2} \\
    \text{subject to} \quad
    & \Prob{\Theta \ge 0.3} \ge 0.9,
    \end{aligned}

with :math:`x \in [0,5]` and :math:`b \in [0.1,5]`.  This is the non-regular
case: the support moves with the parameter.  The canonicalizer selects the
standard normal reference :math:`\Xi=\mathcal{N}(0,1)` and the transport
:math:`T_b(\xi)=b\Phi(\xi)`, so the moving support becomes the fixed reference
range.

The constraint is :math:`(b-0.3)/b \ge 0.9`, hence :math:`b \ge 3`, and the
objective is :math:`b^2/12 + (b/2-x)^2`, minimized at :math:`x=b/2`.  The
solution is therefore :math:`b^\star=3`, :math:`x^\star=1.5`, with objective
:math:`b^2/12 = 0.75` and probability exactly :math:`0.9`.  The test checks
the objective and the constraint to :math:`10^{-10}` and verifies the moving
support sensitivity

.. math::

    \frac{\mathrm{d} \Prob{\Theta \ge 0.3}}{\mathrm{d} b}
    = -\frac{b-0.3}{b^2} + \frac{1}{b}
    = \frac{0.3}{b^2},

which a sensitivity evaluated at a fixed support would miss.  At
:math:`b=3` the flux is :math:`0.3/9 = 1/30`, while a fixed-support
evaluation would return zero.
