%feature("docstring") OTROBOPT::CanonicalizationOptions
R"RAW(Options of the robust problem canonicalizer.

This class is experimental and may change in future releases.

The worst-case policy selects both the reference distribution and the
treatment of the worst-case optimization domain:

- ``AUTO``: use the explicit reference distribution when it is set,
  otherwise the standard fast path when it is available, otherwise the
  standard normal distribution. For a worst-case measure, an explicit domain
  is required unless the standard fast path is used or the reference has a
  bounded range.
- ``STANDARD``: require the standard fast path.
- ``NORMAL``: force the standard normal reference.
- ``USER_REFERENCE``: require ``reference``.
- ``USER_DOMAIN``: use the automatic reference policy and require
  ``worstCaseDomain``.

Notes
-----
The options are set with the accessors of the class:

- ``setReference``: reference distribution, empty for the automatic policy.
  It also selects explicitly the default ``Uniform(-1, 1)`` distribution as
  reference.
- ``setParameterBounds``: bounds on the active distribution parameter, empty
  for no bounds.  A full-dimensional interval is also accepted and filtered
  to the active coordinates.
- ``setParameterStart``: initial distribution parameter, empty for the
  current parameter.  When the parameter is optimized, the point is expressed
  in active coordinates; a full-dimensional point is also accepted and only
  its active coordinates are used.  When the parameter is frozen, a
  full-dimensional point sets the whole distribution parameter, so that the
  frozen distribution is exactly the requested one.
- ``setActiveParameter``: active parameter indices in the full distribution
  parameter.  When it is not set, the default policy of
  :class:`~otrobopt.ParametricDistributionTransformation` is used: the
  trailing dependence or copula block is inactive.
- ``setOptimizeParameter``: whether the distribution parameter is part of the
  decision vector.  Default is True.  When the active parameter is empty, the
  parameter is frozen and the decision vector is reduced to the design
  variables.
- ``setWorstCasePolicy``: worst-case policy, ``CanonicalizationOptions.AUTO``
  by default.
- ``setWorstCaseDomain``: worst-case optimization domain in the reference
  space, empty for the range of the reference distribution.  When the
  distribution parameter is frozen, an empty option domain preserves the
  domain stored by the measure instead; an explicit option domain takes
  precedence over it.  When the parameter is optimized, a domain stored by a
  worst-case measure is rejected, because it is expressed in the original
  distribution coordinates and cannot be transported in general.

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> options = otrobopt.CanonicalizationOptions()
>>> options.setOptimizeParameter(False)
>>> options.setParameterStart([2.0, 0.5])
)RAW"

%feature("docstring") OTROBOPT::RobustOptimizationProblemCanonicalizer
R"RAW(Canonicalize a robust optimization problem over a fixed reference distribution.

This class is experimental and may change in future releases.

The canonical decision vector is :math:`y=(x,s)` where :math:`x` is the
design vector and :math:`s` is the parameter of the distribution
:math:`D_s`. The measure distribution becomes a fixed reference distribution
:math:`\Xi` and the parametric functions are rebuilt as

.. math::

    P(x,s;\xi) = f\bigl(x; T_s(\xi)\bigr),

where :math:`T_s` transports :math:`\Xi` to :math:`D_s`.

When ``options.optimizeParameter`` is false, the distribution parameter is
frozen at ``options.parameterStart`` and the canonical decision vector is
:math:`x` only.

Parameters
----------
problem : :class:`~otrobopt.RobustOptimizationProblem`
    The robust optimization problem to canonicalize.
options : :class:`~otrobopt.CanonicalizationOptions`
    Canonicalization options.

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> thetaDist = ot.Normal(2.0, 0.1)
>>> fBase = ot.SymbolicFunction(['x', 'theta'], ['x * theta'])
>>> f = ot.ParametricFunction(fBase, [1], [1.0])
>>> measure = otrobopt.VarianceMeasure(f, thetaDist)
>>> problem = otrobopt.RobustOptimizationProblem(measure, ot.Function())
>>> canonicalizer = otrobopt.RobustOptimizationProblemCanonicalizer(problem)
>>> canonicalizer.getCanonicalProblem().getDimension()
3
)RAW"

%feature("docstring") OTROBOPT::RobustOptimizationProblemCanonicalizer::getCanonicalProblem
R"RAW(Accessor to the canonical problem.

Returns
-------
problem : :class:`~otrobopt.RobustOptimizationProblem`
    The canonical problem, whose decision vector is :math:`(x,s)`.)RAW"

%feature("docstring") OTROBOPT::RobustOptimizationProblemCanonicalizer::getReferenceDistribution
R"RAW(Accessor to the reference distribution.

Returns
-------
reference : :class:`~openturns.Distribution`
    The fixed reference distribution :math:`\Xi`. When the distribution
    parameter is frozen, this is the frozen distribution :math:`D_{s_0}`.)RAW"

%feature("docstring") OTROBOPT::RobustOptimizationProblemCanonicalizer::getCanonicalObjectiveFunction
R"RAW(Accessor to the canonical objective function.

Returns
-------
objective : :class:`~openturns.Function`
    The objective function of the canonical problem.)RAW"

%feature("docstring") OTROBOPT::RobustOptimizationProblemCanonicalizer::getCanonicalConstraintFunction
R"RAW(Accessor to the canonical inequality constraint function.

Returns
-------
constraint : :class:`~openturns.Function`
    The inequality constraint function of the canonical problem.)RAW"

%feature("docstring") OTROBOPT::RobustOptimizationProblemCanonicalizer::getCanonicalStartingPoint
R"RAW(Accessor to the canonical starting point.

Returns
-------
startingPoint : :class:`~openturns.Point`
    The starting point :math:`(x_0,s_0)` of the canonical problem. When the
    distribution parameter is frozen, only :math:`x_0` is returned.)RAW"

%feature("docstring") OTROBOPT::RobustOptimizationProblemCanonicalizer::getOptimizeParameter
R"RAW(Whether the distribution parameter is optimized.

Returns
-------
optimizeParameter : bool
    True when the canonical decision vector contains the distribution
    parameter.)RAW"

%feature("docstring") OTROBOPT::RobustOptimizationProblemCanonicalizer::usesStandardSpace
R"RAW(Whether the standard fast path is used.

Returns
-------
usesStandardSpace : bool
    True when the reference distribution is the parameter-free standard
    distribution of the measure distribution.)RAW"
