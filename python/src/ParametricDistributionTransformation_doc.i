%feature("docstring") OTROBOPT::ParametricDistributionTransformation
R"RAW(Transformation from a fixed reference distribution to a parameter-dependent distribution.

The input of the transformation is a point :math:`\xi` of the reference
distribution :math:`\Xi`, its output is a point :math:`\theta` of the target
distribution :math:`D_s`, and its parameter is the target distribution
parameter :math:`s = D_s.getParameter()`:

.. math::

    T_s : \mathbb{R}^{n_\xi} \longrightarrow \mathbb{R}^{n_\theta},
    \qquad T_s(\xi) = \text{isoprobabilistic map from } \Xi \text{ to } D_s.

The transformation is rebuilt from :math:`s` whenever the parameter changes.
It therefore does not rely on the parameter vector exposed by the underlying
OpenTURNS transformation.

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
