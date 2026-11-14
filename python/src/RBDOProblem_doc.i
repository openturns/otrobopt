%feature("docstring") OTROBOPT::RBDOProblem
R"RAW(Reliability-based design optimization problem.

This class is experimental and may change in future releases.

The problem couples a scalar classical objective :math:`f(x)` with one joint
chance constraint

.. math::

    \ProbCond{\cD}{G(x;\Theta) \ge 0} \ge \alpha

or the variant defined by the comparison operator of the measure.  The
deterministic bounds, inequality constraints and equality constraints use the
usual :class:`~openturns.OptimizationProblem` protocol.

Available constructors:

    RBDOProblem(*objective, jointChanceMeasure*)

    RBDOProblem()

Parameters
----------
objective : :class:`~openturns.Function`
    Scalar objective function :math:`f(x)`.  Its input dimension defines the
    design dimension.
jointChanceMeasure : :class:`~otrobopt.JointChanceMeasure`
    Joint chance constraint.  Its limit-state function must have the same
    input dimension as the objective and its distribution dimension must
    match the limit-state function parameter dimension.

Notes
-----
The joint chance measure returns the probit-transformed reliability index
:math:`\Phi^{-1}(P)-\Phi^{-1}(\alpha)`, not the probability itself.  The
target probability is available through :meth:`getTargetProbability` and the
comparison operator through :meth:`getComparisonOperator`.

The distribution parameter is frozen by default in the canonical problems
returned by :meth:`getCanonicalProblem` and
:meth:`getCanonicalJointChanceMeasure`.  Use the overloads that accept
:class:`~otrobopt.CanonicalizationOptions` to optimize the active parameter
through the canonical decision vector :math:`(x,s)`.

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> limitState = ot.ParametricFunction(
...     ot.SymbolicFunction(["x", "theta"], ["theta - x"]), [1], [0.0]
... )
>>> measure = otrobopt.JointChanceMeasure(
...     limitState, ot.Normal(), ot.GreaterOrEqual(), 0.9
... )
>>> problem = otrobopt.RBDOProblem(
...     ot.SymbolicFunction(["x"], ["x^2"]), measure
... )
>>> problem.getTargetProbability()
0.9
)RAW"

%feature("docstring") OTROBOPT::RBDOProblem::setJointChanceMeasure
R"RAW(Set the joint chance constraint.

Parameters
----------
jointChanceMeasure : :class:`~otrobopt.JointChanceMeasure`
    Joint chance constraint.  Its limit-state function must have the same
    input dimension as the objective and its distribution dimension must
    match the limit-state function parameter dimension.)RAW"

%feature("docstring") OTROBOPT::RBDOProblem::getJointChanceMeasure
R"RAW(Accessor to the joint chance constraint.

Returns
-------
jointChanceMeasure : :class:`~otrobopt.JointChanceMeasure`
    Joint chance constraint.

Raises
------
ValueError
    If no joint chance constraint is defined.)RAW"

%feature("docstring") OTROBOPT::RBDOProblem::hasJointChanceMeasure
R"RAW(Whether a joint chance constraint is defined.

Returns
-------
hasJointChanceMeasure : bool
    True when a joint chance constraint is defined.)RAW"

%feature("docstring") OTROBOPT::RBDOProblem::getLimitStateFunction
R"RAW(Accessor to the limit-state function of the joint chance constraint.

Returns
-------
limitStateFunction : :class:`~openturns.Function`
    The limit-state function :math:`G(x;\Theta)`, whose parameter is the
    realization of :math:`\Theta`.)RAW"

%feature("docstring") OTROBOPT::RBDOProblem::getDistribution
R"RAW(Accessor to the distribution of the joint chance constraint.

Returns
-------
distribution : :class:`~openturns.Distribution`
    Distribution of the uncertain vector :math:`\Theta`.)RAW"

%feature("docstring") OTROBOPT::RBDOProblem::getTargetProbability
R"RAW(Accessor to the target probability of the joint chance constraint.

Returns
-------
targetProbability : float
    The level :math:`\alpha` of the joint chance constraint.)RAW"

%feature("docstring") OTROBOPT::RBDOProblem::getComparisonOperator
R"RAW(Accessor to the comparison operator of the joint chance constraint.

Returns
-------
comparisonOperator : :class:`~openturns.ComparisonOperator`
    The operator that defines the joint chance event.)RAW"

%feature("docstring") OTROBOPT::RBDOProblem::check
R"RAW(Check the problem contract.

Raises
------
ValueError
    If the objective is missing or not scalar, if the joint chance constraint
    is missing, if the dimensions do not match, or if the deterministic
    constraints or bounds are inconsistent with the design dimension.)RAW"

%feature("docstring") OTROBOPT::RBDOProblem::GetDefaultCanonicalizationOptions
R"RAW(Default canonicalization options of the specialized problem.

The distribution parameter is frozen by default.  Use the overloads of
:meth:`getCanonicalizer`, :meth:`getCanonicalProblem`,
:meth:`getCanonicalJointChanceMeasure`, :meth:`getCanonicalDeterministicConstraint`
and :meth:`getCanonicalStartingPoint` that accept
:class:`~otrobopt.CanonicalizationOptions` to optimize the active parameter.

Returns
-------
options : :class:`~otrobopt.CanonicalizationOptions`
    Canonicalization options with ``optimizeParameter`` set to False.)RAW"

%feature("docstring") OTROBOPT::RBDOProblem::getCanonicalizer
R"RAW(Accessor to the canonicalizer of the problem.

Returns
-------
canonicalizer : :class:`~otrobopt.RobustOptimizationProblemCanonicalizer`
    The canonicalizer built from the generic robust problem, with the default
    options.)RAW"

%feature("docstring") OTROBOPT::RBDOProblem::getCanonicalProblem
R"RAW(Accessor to the solver-ready canonical problem.

The canonical inequality constraint gathers the joint chance index as its
first output and the deterministic inequality constraints as the following
outputs.  The canonical joint chance measure itself remains available through
:meth:`getCanonicalJointChanceMeasure`.

Returns
-------
problem : :class:`~otrobopt.RobustOptimizationProblem`
    The canonical problem, whose decision vector is :math:`(x,s)` when the
    parameter is optimized and :math:`x` otherwise.)RAW"

%feature("docstring") OTROBOPT::RBDOProblem::getCanonicalJointChanceMeasure
R"RAW(Accessor to the canonical joint chance measure.

Returns
-------
measure : :class:`~otrobopt.JointChanceMeasure`
    The joint chance measure rebuilt on the canonical limit-state function
    and the reference distribution.  The measure still carries the target
    probability and the comparison operator of the original problem.)RAW"

%feature("docstring") OTROBOPT::RBDOProblem::getCanonicalDeterministicConstraint
R"RAW(Accessor to the canonical deterministic inequality constraint.

Returns
-------
constraint : :class:`~openturns.Function`
    The deterministic inequality constraints lifted to the canonical decision
    vector, or an empty function when the problem has none.)RAW"

%feature("docstring") OTROBOPT::RBDOProblem::getCanonicalStartingPoint
R"RAW(Accessor to the canonical starting point.

Returns
-------
startingPoint : :class:`~openturns.Point`
    The starting point :math:`(x_0,s_0)` of the canonical problem.  When the
    distribution parameter is frozen, only :math:`x_0` is returned.)RAW"
