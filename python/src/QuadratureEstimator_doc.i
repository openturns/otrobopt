%feature("docstring") OTROBOPT::QuadratureEstimator
R"RAW(Deterministic quadrature estimator of a system-event probability.

For a continuous distribution :math:`\cD` of density :math:`p`, the
probability is the Lebesgue integral

.. math::

    P(x) = \int_{\mathrm{Supp}(\cD)}
    \varphi\left(\mathbf{1}_{G_1(x, \theta) \ge 0}, \dots,
    \mathbf{1}_{G_m(x, \theta) \ge 0}\right) p(\theta)\, d\theta.

The density threshold only truncates the tails of this continuous integrand.

For a discrete distribution, the probability is the exact sum over the
support,

.. math::

    P(x) = \sum_{\theta_i \in \mathrm{Supp}(\cD)} p_i\,
    \varphi\left(\mathbf{1}_{G_1(x, \theta_i) \ge 0}, \dots,
    \mathbf{1}_{G_m(x, \theta_i) \ge 0}\right),

and no threshold is applied: dropping a low-probability atom would change the
probability instead of truncating a density tail.

The estimator is deterministic and has no geometry.  Its
:py:meth:`~otrobopt.ProbabilityEstimator.getEstimationError` is NaN because the
integration algorithm does not return a quadrature gap.

Parameters
----------
algorithm : :py:class:`openturns.IntegrationAlgorithm`
    Integration algorithm used for the continuous branch.  It defaults to an
    iterated Gauss-Kronrod quadrature.

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> f = ot.ParametricFunction(
...     ot.SymbolicFunction(["x", "theta"], ["x - theta"]), [1], [0.0]
... )
>>> predicate = ot.SymbolicFunction(["y0"], ["y0"])
>>> estimator = otrobopt.QuadratureEstimator()
>>> abs(estimator.getSafeProbability(predicate, f, ot.Normal(), [0.0]) - 0.5) < 1e-10
True
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::QuadratureEstimator::setIntegrationAlgorithm
"Integration algorithm accessor.

Parameters
----------
algorithm : :py:class:`openturns.IntegrationAlgorithm`
    Integration algorithm used for the continuous branch."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::QuadratureEstimator::getIntegrationAlgorithm
"Integration algorithm accessor.

Returns
-------
algorithm : :py:class:`openturns.IntegrationAlgorithm`
    Integration algorithm used for the continuous branch."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::QuadratureEstimator::setPDFThreshold
"Density threshold accessor.

Parameters
----------
threshold : float
    Nonnegative threshold below which the continuous integrand is zero.  It
    has no effect on the discrete branch."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::QuadratureEstimator::getPDFThreshold
"Density threshold accessor.

Returns
-------
threshold : float
    Nonnegative threshold below which the continuous integrand is zero."
