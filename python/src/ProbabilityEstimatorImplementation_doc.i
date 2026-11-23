%feature("docstring") OTROBOPT::ProbabilityEstimatorImplementation
R"RAW(Implementation base class of the system-event probability estimators.

This class is the implementation layer of :py:class:`~otrobopt.ProbabilityEstimator`.
It is exposed so that the interface/implementation machinery can convert the
concrete estimators, but it is not meant to be instantiated by users.

The event is a Boolean predicate :math:`\varphi` of the indicators of
:math:`m` limit-state components.  With :math:`G_j(x, \theta)` the :math:`j`-th
component at the design point :math:`x` for the realization :math:`\theta` of
the distribution :math:`\cD`, the indicator is
:math:`\mathbf{1}_{G_j(x, \theta) \ge 0}` and the event is

.. math::

    \varphi\left(\mathbf{1}_{G_1(x, \theta) \ge 0}, \dots,
    \mathbf{1}_{G_m(x, \theta) \ge 0}\right) = 1.

The predicate is a :py:class:`openturns.Function` with :math:`m` inputs, one
output and no parameter; it must return 0 or 1.  The limit state is a
parametric :py:class:`openturns.Function` whose parameter is :math:`\theta`.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ProbabilityEstimatorImplementation::getSafeProbability
R"RAW(Probability of the event at a design point.

Parameters
----------
predicate : :py:class:`openturns.Function`
    Boolean predicate of the :math:`m` limit-state indicators.
limitState : :py:class:`openturns.Function`
    Parametric limit-state function with :math:`m` outputs, whose parameter is
    the realization of the distribution.
distribution : :py:class:`openturns.Distribution`
    Distribution of the limit-state parameter.
designPoint : sequence of float
    Design point at which the limit state is evaluated.

Returns
-------
probability : float
    Probability that the predicate is true, in :math:`[0, 1]`.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ProbabilityEstimatorImplementation::getEstimationError
R"RAW(Error estimate of the probability.

Parameters
----------
predicate : :py:class:`openturns.Function`
    Boolean predicate of the :math:`m` limit-state indicators.
limitState : :py:class:`openturns.Function`
    Parametric limit-state function with :math:`m` outputs.
distribution : :py:class:`openturns.Distribution`
    Distribution of the limit-state parameter.
designPoint : sequence of float
    Design point at which the limit state is evaluated.

Returns
-------
error : float
    Error estimate in probability units, or NaN when the estimator has no
    error estimate for this call.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ProbabilityEstimatorImplementation::hasGeometry
R"RAW(Whether the estimator provides a geometry.

Returns
-------
hasGeometry : bool
    True when design points and probability bounds are available, as for the
    reliability estimator.  False for the quadrature and experiment
    estimators.
)RAW"
