%feature("docstring") OTROBOPT::ParameterAsInputFunction
R"RAW(Function whose parameter is exposed as input coordinates.

If :math:`f` has input dimension :math:`d`, output dimension :math:`p` and
parameter dimension :math:`q`, then the constructed function :math:`H` has
input dimension :math:`d+q`, output dimension :math:`p` and no parameter:

.. math::

    H(u, \theta) = f_\theta(u)

where :math:`f_\theta` is the original function evaluated with parameter
:math:`\theta`.

The gradient of :math:`H` is the concatenation of the gradient of
:math:`f_\theta` with respect to :math:`u` and of its gradient with respect
to :math:`\theta`.

Parameters
----------
function : :class:`~openturns.Function`
    A function whose parameter is exposed as input coordinates.

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> f = ot.ParametricFunction(
...     ot.SymbolicFunction(['x', 'theta'], ['x * theta']), [1], [3.0]
... )
>>> h = otrobopt.ParameterAsInputFunction(f)
>>> h.getInputDimension()
2
>>> h.getParameterDimension()
0
>>> h([2.0, 3.0])[0]
6.0
>>> h([2.0, 5.0])[0]
10.0
)RAW"
