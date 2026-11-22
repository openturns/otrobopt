%feature("docstring") OTROBOPT::ProbabilityEstimator
R"RAW(Handle to an estimator of the probability of a system event.

The handle owns its estimator by a shared pointer, so that a chance measure can
be copied and persisted without slicing the concrete estimator.  The concrete
estimators derive from ``ProbabilityEstimatorImplementation`` and are accepted
wherever a :py:class:`~otrobopt.ProbabilityEstimator` is expected.

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
The estimator is stateless: every call receives the predicate, the limit
state, the distribution and the design point.

The probability is returned before the probit transform applied by the chance
measure.  :py:meth:`getEstimationError` returns NaN when the estimator has no
error estimate for the requested call.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ProbabilityEstimator::getSafeProbability
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

%feature("docstring") OTROBOPT::ProbabilityEstimator::getEstimationError
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

%feature("docstring") OTROBOPT::ProbabilityEstimator::hasGeometry
R"RAW(Whether the estimator provides a geometry.

Returns
-------
hasGeometry : bool
    True when design points and probability bounds are available, as for the
    reliability estimator.  False for the quadrature and experiment
    estimators.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ProbabilityEstimator::getComponentReliabilityIndexes
R"RAW(Signed reliability index of each limit-state component.

Only the estimators based on a reliability analysis provide a per-component
geometry; the others raise an exception.

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
indexes : :py:class:`openturns.Point`
    One signed index per limit-state component.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ProbabilityEstimator::getStandardDesignPoints
R"RAW(Standard-space design points, grouped by limit-state component.

Only the estimators based on a reliability analysis provide a per-component
geometry; the others raise an exception.

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
points : :py:class:`openturns.Sample`
    Design points, one per linearized half-space, grouped by component.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ProbabilityEstimator::getDesignPointsNumber
R"RAW(Number of standard-space design points of each limit-state component.

Only the estimators based on a reliability analysis provide a per-component
geometry; the others raise an exception.

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
number : :py:class:`openturns.Indices`
    Number of design points of each component.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ProbabilityEstimator::getProbabilityBounds
R"RAW(Bounds of the probability of the linearized event.

Only the estimators based on a reliability analysis provide a per-component
geometry; the others raise an exception.

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
bounds : :py:class:`openturns.Interval`
    Lower and upper bounds of the probability.
)RAW"
