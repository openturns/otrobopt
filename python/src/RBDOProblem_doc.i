%feature("docstring") OTROBOPT::RBDOProblem
"Reliability-Based Design Optimization problem.

Defines an RBDO problem with objective, probabilistic and deterministic
constraints, bounds, and the design/parameter vector structure.

The standard RBDO formulation solved by :class:`RIAAlgorithm`, :class:`PMAAlgorithm`,
and :class:`SORAAlgorithm` is:

.. math::

   \\min_{d} \\quad f(d, p)

   \\text{subject to} \\quad P[g(d, X, Z) \\leq 0] \\leq \\Phi(-\\beta)

where:

- :math:`d` is the vector of design variables,
- :math:`p` is the vector of fixed parameters,
- :math:`[d, p]` is the combined parameter vector whose structure is defined by
  `activeDesignIndices` and `activeParameterIndices`,
- :math:`X \\sim \\text{distributionX}` are random variables (which may depend on
  the design variables),
- :math:`Z \\sim \\text{distributionZ}` are fixed random variables,
- :math:`g(d, X, Z)` is the limit state function (`constraintFunction`), where
  :math:`g \\leq 0` defines the failure event,
- :math:`\\beta` is the target reliability index (`targetBeta`),
- :math:`\\Phi` is the standard normal cumulative distribution function, so the
  target failure probability is :math:`P_f = \\Phi(-\\beta)`.

When `constraintOperator` is :class:`openturns.Greater`, the constraint reads
:math:`P[g(d, X, Z) \\geq 0] \\leq \\Phi(-\\beta)` instead.

Parameters
----------
objective : :py:class:`openturns.Function`
    Objective function f(d, p)
constraintFunction : :py:class:`openturns.Function`
    Probabilistic constraint function g(d, X, Z)
distributionX : :py:class:`openturns.Distribution`
    Distribution of X
distributionZ : :py:class:`openturns.Distribution`
    Distribution of Z (fixed)
targetBeta : float
    Target reliability index
constraintOperator : :py:class:`openturns.ComparisonOperator`
    Comparison operator for failure event (Less: P[g<=0], Greater: P[g>=0])
activeDesignIndices : sequence of int
    Indices of d in [d, p]
activeParameterIndices : sequence of int
    Indices of p in [d, p]

See also
--------
RIAAlgorithm, PMAAlgorithm, SORAAlgorithm"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RBDOProblem::setObjective
"Objective function accessor.

Parameters
----------
objective : :py:class:`openturns.Function`
    Objective function f(d, p)"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RBDOProblem::getObjective
"Objective function accessor.

Returns
-------
objective : :py:class:`openturns.Function`"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RBDOProblem::setTargetBeta
"Target reliability index accessor.

Parameters
----------
targetBeta : float
    Target reliability index"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RBDOProblem::getTargetBeta
"Target reliability index accessor.

Returns
-------
targetBeta : float"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RBDOProblem::setConstraintFunction
"Probabilistic constraint function accessor.

Parameters
----------
function : :py:class:`openturns.Function`
    Limit state function g(d, X, Z)"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RBDOProblem::setDistributionX
"Distribution of X accessor.

Parameters
----------
distribution : :py:class:`openturns.Distribution`
    Distribution of random variables X"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RBDOProblem::buildJointDistribution
"Build the joint distribution of (X, Z).

Returns
-------
jointDistribution : :py:class:`openturns.Distribution`"
