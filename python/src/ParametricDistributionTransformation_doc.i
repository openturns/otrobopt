%feature("docstring") OTROBOPT::ParametricDistributionTransformation
R"RAW(Transformation from a fixed reference distribution to a parameter-dependent distribution.

The input of the transformation is a point :math:`\xi` of the reference
distribution :math:`\Xi`, its output is a point :math:`\theta` of the target
distribution :math:`D_s`, and its parameter is the target distribution
parameter :math:`s = D_s.getParameter()`:

.. math::

    T_s : \Rset^{n_\xi} \longrightarrow \Rset^{n_\theta},
    \qquad T_s(\xi) = \text{isoprobabilistic map from } \Xi \text{ to } D_s.

The transformation is rebuilt from :math:`s` whenever the parameter changes.
It therefore does not rely on the parameter vector exposed by the underlying
OpenTURNS transformation.

The parameter obeys the :class:`~openturns.CovarianceModel` full/active
convention.  The full parameter vector is the distribution parameter
``D_s.getParameter()``.  The active parameter vector is the subset of
coordinates exposed to the optimizer: ``getParameter``, ``setParameter``,
``getParameterDescription`` and ``getParameterDimension`` operate on the
active coordinates, while ``getFullParameter``, ``setFullParameter`` and
``getFullParameterDescription`` operate on the full vector.  By default, the
dependence parameters are inactive because OpenTURNS does not provide their
analytic parameter derivative: this covers the correlation coefficients
``R_i_j`` wherever they appear in the full parameter, and the trailing copula
block when the distribution exposes it.  All other parameters are active.  Use
``setActiveParameter`` to change the selection.

When no reference is given, the standard distribution of :math:`D_s` is used
when it is parameter-free. Otherwise the standard normal distribution in the
target dimension is used.

Parameters
----------
distribution : :class:`~openturns.Distribution`
    The parameter-dependent target distribution.
reference : :class:`~openturns.Distribution`
    The fixed reference distribution. When omitted, the automatic policy
    described above is used.

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> T = otrobopt.ParametricDistributionTransformation(ot.Normal(2.0, 3.0))
>>> T.getParameter()[0]
2.0
>>> T([0.0])[0]
2.0
>>> T.setParameter([5.0, 7.0])
>>> T([0.0])[0]
5.0
>>> T([1.0])[0]
12.0
)RAW"

%feature("docstring") OTROBOPT::ParametricDistributionTransformation::getDistribution
R"RAW(Accessor to the target distribution.

Returns
-------
distribution : :class:`~openturns.Distribution`
    A copy of the target distribution at the current parameter.)RAW"

%feature("docstring") OTROBOPT::ParametricDistributionTransformation::getReferenceDistribution
R"RAW(Accessor to the reference distribution.

Returns
-------
reference : :class:`~openturns.Distribution`
    A copy of the reference distribution.)RAW"

%feature("docstring") OTROBOPT::ParametricDistributionTransformation::usesStandardSpace
R"RAW(Whether the standard fast path is used.

Returns
-------
usesStandardSpace : bool
    True when the reference is the parameter-free standard distribution of
    the target distribution and the inverse isoprobabilistic transformation
    is used.)RAW"

%feature("docstring") OTROBOPT::ParametricDistributionTransformation::getParameter
R"RAW(Accessor to the active parameter.

Returns
-------
parameter : :class:`~openturns.Point`
    The values of the active coordinates of the full distribution parameter.)RAW"

%feature("docstring") OTROBOPT::ParametricDistributionTransformation::setParameter
R"RAW(Accessor to the active parameter.

Parameters
----------
parameter : sequence of float
    The new values of the active coordinates.  The full parameter is rebuilt
    from the current values, the active coordinates are overwritten, and the
    result is validated by the distribution.  An invalid value raises an
    exception instead of being clipped.)RAW"

%feature("docstring") OTROBOPT::ParametricDistributionTransformation::getParameterDescription
R"RAW(Accessor to the active parameter description.

Returns
-------
description : :class:`~openturns.Description`
    The descriptions of the active coordinates.)RAW"

%feature("docstring") OTROBOPT::ParametricDistributionTransformation::getParameterDimension
R"RAW(Accessor to the active parameter dimension.

Returns
-------
dimension : int
    The number of active parameter coordinates.)RAW"

%feature("docstring") OTROBOPT::ParametricDistributionTransformation::getFullParameter
R"RAW(Accessor to the full parameter.

Returns
-------
parameter : :class:`~openturns.Point`
    The full distribution parameter, including the inactive dependence or
    copula block.)RAW"

%feature("docstring") OTROBOPT::ParametricDistributionTransformation::setFullParameter
R"RAW(Accessor to the full parameter.

Parameters
----------
parameter : sequence of float
    The new full distribution parameter.  The value is validated by the
    distribution; an invalid value raises an exception.)RAW"

%feature("docstring") OTROBOPT::ParametricDistributionTransformation::getFullParameterDescription
R"RAW(Accessor to the full parameter description.

Returns
-------
description : :class:`~openturns.Description`
    The descriptions of all the coordinates of the full parameter.)RAW"

%feature("docstring") OTROBOPT::ParametricDistributionTransformation::getActiveParameter
R"RAW(Accessor to the active parameter indices.

Returns
-------
active : :class:`~openturns.Indices`
    The indices of the active coordinates in the full parameter vector.)RAW"

%feature("docstring") OTROBOPT::ParametricDistributionTransformation::setActiveParameter
R"RAW(Accessor to the active parameter indices.

Parameters
----------
active : sequence of int
    The indices of the active coordinates in the full parameter vector.  The
    indices must be unique and in the range of the full parameter.  An empty
    sequence freezes the parameter.)RAW"

%feature("docstring") OTROBOPT::ParametricDistributionTransformation::GetDefaultActiveParameter
R"RAW(Default active parameter indices for a distribution.

The default keeps all parameters active except the dependence parameters:
the correlation coefficients ``R_i_j`` wherever they appear in the full
parameter, and the trailing copula block when the distribution exposes it
through ``Distribution.getParametersCollection``.  This excludes the
parameters for which OpenTURNS does not provide an analytic parameter
derivative.

Parameters
----------
distribution : :class:`~openturns.Distribution`
    The distribution whose parameter vector is inspected.

Returns
-------
active : :class:`~openturns.Indices`
    The default active parameter indices.)RAW"
