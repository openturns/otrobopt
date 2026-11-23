%feature("docstring") OTROBOPT::ReliabilityEstimator
R"RAW(First-order estimator of the probability of a Boolean system event.

The estimator analyses each of the :math:`m` components of the limit-state
function by FORM, optionally restarted to collect several design points
(MultiFORM).  The failure event of component :math:`j` is linearized in the
standard space as :math:`\{a_j^\top u \ge \beta_j\}`, where :math:`a_j` is the
unit normal obtained from the design point and :math:`\beta_j` is its signed
distance to the origin.  The sign carries the orientation, so that a component
safe event with a probability larger than one half is represented without
clamping.

The system event is the Boolean predicate applied to the :math:`m` linearized
component indicators.  Its probability is combined exactly over the
:math:`2^N` atoms of the :math:`N` linearized half-spaces; an atom probability
is the orthant probability of the standard normal law with correlation matrix
:math:`R_{ij} = a_i^\top a_j`.  The exact combination is attempted only when
:math:`2^N` does not exceed :meth:`getMaximumCombinationNumber`.  Above that
bound the estimator returns the midpoint of the first-order bounds obtained
from the component marginals, and reports their half-width as the estimation
error: the result is then an explicitly bounded estimate, never a silently
truncated exact sum.

The analytical combination requires a normal standard space, that is a
distribution whose standard distribution is the standard normal law.  For any
other distribution the system algorithm must be ``SIMULATION``, which samples
the distribution, applies the predicate to the component indicators and reports
the binomial standard error.

Parameters
----------
componentAlgorithm : str
    ``FORM`` or ``MULTIFORM``.  ``MULTIFORM`` restarts the nearest-point search
    to collect the competing design points of a component.
systemAlgorithm : str
    ``ANALYTICAL`` (first-order combination) or ``SIMULATION``.

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> limitState = ot.ParametricFunction(
...     ot.SymbolicFunction(["x", "theta"], ["x - theta"]), [1], [0.0]
... )
>>> predicate = ot.SymbolicFunction(["y0"], ["y0"])
>>> estimator = otrobopt.ReliabilityEstimator("FORM", "ANALYTICAL")
>>> probability = estimator.getSafeProbability(
...     predicate, limitState, ot.Normal(), [1.0]
... )
>>> abs(probability - ot.Normal().computeCDF(1.0)) < 1e-6
True
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::getComponentAlgorithm
"Component algorithm accessor.

Returns
-------
algorithm : str
    ``FORM`` or ``MULTIFORM``."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::setComponentAlgorithm
"Component algorithm accessor.

Parameters
----------
algorithm : str
    ``FORM`` or ``MULTIFORM``.  ``MULTIFORM`` restarts the nearest-point search
    to collect the competing design points of a component."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::getSystemAlgorithm
"System algorithm accessor.

Returns
-------
algorithm : str
    ``ANALYTICAL`` or ``SIMULATION``."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::setSystemAlgorithm
"System algorithm accessor.

Parameters
----------
algorithm : str
    ``ANALYTICAL`` (first-order combination in a normal standard space) or
    ``SIMULATION`` (sampling fallback for any distribution)."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::getSolver
"Nearest-point solver accessor.

Returns
-------
solver : :py:class:`openturns.OptimizationAlgorithm`
    Solver of the per-component nearest-point searches."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::setSolver
"Nearest-point solver accessor.

Parameters
----------
solver : :py:class:`openturns.OptimizationAlgorithm`
    Solver of the per-component nearest-point searches."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::getMaximumDesignPointsNumber
"Maximum number of design points of one component.

Returns
-------
number : int
    Upper bound on the number of design points collected by ``MULTIFORM``."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::setMaximumDesignPointsNumber
"Maximum number of design points of one component.

Parameters
----------
number : int
    Upper bound on the number of design points collected by ``MULTIFORM``."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::getMaximumCombinationNumber
"Maximum number of atoms of the exact combination.

Returns
-------
number : int
    Largest accepted :math:`2^N`, where :math:`N` is the number of linearized
    half-spaces.  Above that bound the estimator returns the midpoint of the
    first-order bounds."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::setMaximumCombinationNumber
"Maximum number of atoms of the exact combination.

Parameters
----------
number : int
    Largest accepted :math:`2^N`, where :math:`N` is the number of linearized
    half-spaces.  Above that bound the estimator returns the midpoint of the
    first-order bounds."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::getBetaTarget
"Nominal reliability index used to seed the inner analyses.

Returns
-------
index : float
    Positive nominal index that scales the starting points of the
    nearest-point searches.  It does not change the computed design points."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::setBetaTarget
"Nominal reliability index used to seed the inner analyses.

Parameters
----------
index : float
    Positive nominal index that scales the starting points of the
    nearest-point searches.  It does not change the computed design points."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::getSampleSize
"Sample size of the simulation system algorithm.

Returns
-------
size : int
    Number of samples used by the ``SIMULATION`` system algorithm."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::setSampleSize
"Sample size of the simulation system algorithm.

Parameters
----------
size : int
    Number of samples used by the ``SIMULATION`` system algorithm."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::getComponentReliabilityIndexes
R"RAW(Signed reliability index of each limit-state component.

The index of component :math:`j` is :math:`\Phi^{-1}(P_j)`, where
:math:`P_j` is the first-order probability of its linearized safe event.  It is
negative when that probability is smaller than one half.

Parameters
----------
predicate : :py:class:`openturns.Function`
    Boolean predicate of the component indicators.
limitState : :py:class:`openturns.Function`
    Parametric limit-state function.
distribution : :py:class:`openturns.Distribution`
    Distribution of the limit-state parameter.
designPoint : sequence of float
    Design point at which the limit state is evaluated.

Returns
-------
indexes : :py:class:`openturns.Point`
    One signed index per component.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::getStandardDesignPoints
R"RAW(Standard-space design points, grouped by limit-state component.

Each row is the standard-space design point of one linearized half-space.  The
rows are grouped by component, in the order of the limit-state outputs; the
number of rows of each group is given by
:meth:`getDesignPointsNumber`.

Parameters
----------
predicate : :py:class:`openturns.Function`
    Boolean predicate of the component indicators.
limitState : :py:class:`openturns.Function`
    Parametric limit-state function.
distribution : :py:class:`openturns.Distribution`
    Distribution of the limit-state parameter.
designPoint : sequence of float
    Design point at which the limit state is evaluated.

Returns
-------
points : :py:class:`openturns.Sample`
    Design points, one per linearized half-space.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::getDesignPointsNumber
R"RAW(Number of standard-space design points of each limit-state component.

Parameters
----------
predicate : :py:class:`openturns.Function`
    Boolean predicate of the component indicators.
limitState : :py:class:`openturns.Function`
    Parametric limit-state function.
distribution : :py:class:`openturns.Distribution`
    Distribution of the limit-state parameter.
designPoint : sequence of float
    Design point at which the limit state is evaluated.

Returns
-------
number : :py:class:`openturns.Indices`
    Number of design points of each component, in the order of the limit-state
    outputs.
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ReliabilityEstimator::getProbabilityBounds
R"RAW(Bounds of the probability of the linearized event.

The bounds are those returned by the bounded combination: the interval is
degenerate when the exact combination was affordable, and is the first-order
bounds from the component marginals otherwise.

Parameters
----------
predicate : :py:class:`openturns.Function`
    Boolean predicate of the component indicators.
limitState : :py:class:`openturns.Function`
    Parametric limit-state function.
distribution : :py:class:`openturns.Distribution`
    Distribution of the limit-state parameter.
designPoint : sequence of float
    Design point at which the limit state is evaluated.

Returns
-------
bounds : :py:class:`openturns.Interval`
    Lower and upper bounds of the probability.
)RAW"
