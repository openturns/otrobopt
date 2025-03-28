%feature("docstring") OTROBOPT::QuantileMeasure
R"RAW(Quantile measure function.

It is built from a parametric function :math:`f` and a distribution :math:`\cD` of density function :math:`p`.

.. math::

    M_{f, \cD, \alpha}(x) = \inf \{ s \in \Rset \: | \: \Pset (f(x, \Theta) \leq s) \geq \alpha \}

Parameters
----------
function : :py:class:`openturns.Function`
    Parametric function
distribution : :py:class:`openturns.Distribution`
    Distribution of parameters
alpha : float, in :math:`[0,1]`
    Quantile level

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> thetaDist = ot.Normal(2.0, 0.1)
>>> f_base = ot.SymbolicFunction(['x', 'theta'], ['x*theta'])
>>> f = ot.ParametricFunction(f_base, [1], [1.0])
>>> measure = otrobopt.QuantileMeasure(f, thetaDist, 0.99)
>>> x = [1.0]
>>> print(measure(x))
[2.23264])RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::QuantileMeasure::setAlpha
"Quantile level accessor.

Parameters
----------
alpha : float
    Quantile level"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::QuantileMeasure::getAlpha
"Quantile level accessor.

Returns
-------
alpha : float
    Quantile level"
