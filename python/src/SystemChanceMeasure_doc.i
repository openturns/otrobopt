%feature("docstring") OTROBOPT::SystemChanceMeasure
R"RAW(System chance measure function.

It is built from a parametric limit-state function :math:`G`, a Boolean
predicate :math:`\varphi` of the indicators of its :math:`m` components, and a
distribution :math:`\cD` of the parameter :math:`\Theta`.  At the design point
:math:`x`, the safe event is

.. math::

    E(x) = \left\{\varphi\left(\mathbf{1}_{G_1(x, \Theta) \ge 0}, \dots,
    \mathbf{1}_{G_m(x, \Theta) \ge 0}\right) = 1\right\}.

The predicate is a :py:class:`openturns.Function` with :math:`m` inputs, one
output and no parameter; it must return 0 or 1.  It makes the event an explicit
part of the measure, independently of how its probability is estimated.

The probability :math:`P(x)` of :math:`E(x)` is delegated to a
:py:class:`~otrobopt.ProbabilityEstimator` owned by the measure.  The measure
returns the probit-constraint

.. math::

    M_{G, \varphi, \cD, \alpha}(x) = \Phi^{-1}(P(x)) - \Phi^{-1}(\alpha)

for a :py:class:`openturns.Less` operator, and its opposite for any other
operator.  As in :py:class:`~otrobopt.JointChanceMeasure`, :math:`\alpha` is
the target probability of the safe event and the comparison operator only
orients the constraint.

Parameters
----------
limitState : :py:class:`openturns.Function`
    Parametric limit-state function with :math:`m` outputs, whose parameter is
    the realization of the distribution.
eventPredicate : :py:class:`openturns.Function`
    Boolean predicate of the :math:`m` limit-state indicators.
distribution : :py:class:`openturns.Distribution`
    Distribution of the limit-state parameter.
operator : :py:class:`openturns.ComparisonOperator`
    Comparison operator in front of :math:`\alpha`.
alpha : float in :math:`(0,1)`
    Target probability of the safe event.

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> limitState = ot.ParametricFunction(
...     ot.SymbolicFunction(["x", "theta"], ["x - theta"]), [1], [0.0]
... )
>>> predicate = ot.SymbolicFunction(["y0"], ["y0"])
>>> measure = otrobopt.SystemChanceMeasure(
...     limitState, predicate, ot.Normal(), ot.GreaterOrEqual(), 0.9
... )
>>> probability = measure.getSafeProbability([0.0])
>>> abs(probability - 0.5) < 1e-12
True
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::getSafeProbability
R"RAW(Probability of the stored event at the given design point.

Parameters
----------
designPoint : sequence of float
    Design point at which the limit state is evaluated.

Returns
-------
probability : float
    Probability of the safe event, in :math:`[0, 1]`.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::getReliabilityIndex
R"RAW(Signed reliability index of the stored event.

The index is :math:`\beta = \Phi^{-1}(P)`, where :math:`P` is the probability
of the safe event.  It is negative when the safe event is less likely than its
complement.

Parameters
----------
designPoint : sequence of float
    Design point at which the limit state is evaluated.

Returns
-------
index : float
    Signed reliability index.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::getEstimationError
R"RAW(Error estimate of the stored event probability.

Parameters
----------
designPoint : sequence of float
    Design point at which the limit state is evaluated.

Returns
-------
error : float
    Error estimate in probability units, or NaN when the estimator has no
    error estimate for this call.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::setEventPredicate
"Event predicate accessor.

Parameters
----------
predicate : :py:class:`openturns.Function`
    Boolean predicate of the :math:`m` limit-state indicators."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::getEventPredicate
"Event predicate accessor.

Returns
-------
predicate : :py:class:`openturns.Function`
    Boolean predicate of the :math:`m` limit-state indicators."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::setEstimator
"Probability estimator accessor.

Parameters
----------
estimator : :py:class:`~otrobopt.ProbabilityEstimator`
    Estimator of the event probability.  The concrete estimators are accepted
    directly: :py:class:`~otrobopt.QuadratureEstimator` and
    :py:class:`~otrobopt.ExperimentEstimator`."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::getEstimator
"Probability estimator accessor.

Returns
-------
estimator : :py:class:`~otrobopt.ProbabilityEstimator`
    Estimator of the event probability."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::setAlpha
"Target probability accessor.

Parameters
----------
alpha : float in :math:`(0,1)`
    Target probability of the safe event."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::getAlpha
"Target probability accessor.

Returns
-------
alpha : float in :math:`(0,1)`
    Target probability of the safe event."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::setOperator
"Comparison operator accessor.

Parameters
----------
op : :py:class:`openturns.ComparisonOperator`
    Comparison operator."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::getOperator
"Comparison operator accessor.

Returns
-------
op : :py:class:`openturns.ComparisonOperator`
    Comparison operator."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::hasGeometry
R"RAW(Whether the estimator provides a geometry.

Returns
-------
hasGeometry : bool
    True when the estimator exposes design points and probability bounds, as
    the reliability estimator does.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::getComponentReliabilityIndexes
R"RAW(Signed reliability index of each limit-state component.

Parameters
----------
designPoint : sequence of float
    Design point at which the limit state is evaluated.

Returns
-------
indexes : :py:class:`openturns.Point`
    One signed index per limit-state component.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::getStandardDesignPoints
R"RAW(Standard-space design points, grouped by limit-state component.

Parameters
----------
designPoint : sequence of float
    Design point at which the limit state is evaluated.

Returns
-------
points : :py:class:`openturns.Sample`
    Design points, one per linearized half-space, grouped by component.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::getDesignPointsNumber
R"RAW(Number of standard-space design points of each limit-state component.

Parameters
----------
designPoint : sequence of float
    Design point at which the limit state is evaluated.

Returns
-------
number : :py:class:`openturns.Indices`
    Number of design points of each component.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SystemChanceMeasure::getProbabilityBounds
R"RAW(Bounds of the probability of the linearized event.

Parameters
----------
designPoint : sequence of float
    Design point at which the limit state is evaluated.

Returns
-------
bounds : :py:class:`openturns.Interval`
    Lower and upper bounds of the probability.
)RAW"
