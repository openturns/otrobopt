.. _robust-optimization-theory:

Robust optimization theory
==========================

.. currentmodule:: otrobopt

This page introduces the notation and the problem classes of robust
optimization in otrobopt.  The mathematical core is in
:doc:`invariance`: the law-determined property, the pushforward identity,
the invariance theorem, the parameter-dependent construction and the
discretization.  The measure catalogue is in :doc:`robust_optimization`,
the specialized algorithms in :doc:`algorithms`, the validation benchmarks
in :doc:`benchmarks`, and the references in :doc:`bibliography`.  The class
reference is in the :doc:`../user_manual/user_manual`.

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
  aggregated first, as explained in
  :ref:`robust-optimization-parameter-dependence`.
* All moments, quantiles and probabilities that appear are finite, and the
  parameter qualifications (positive scales, positive degrees of freedom,
  positive definite correlation matrices) are enforced by the user bounds.
