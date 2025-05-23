%feature("docstring") OTROBOPT::VarianceMeasure
R"RAW(Variance measure function.

It is built from a parametric function :math:`f` and a distribution :math:`\cD` of density function :math:`p`.

In case of a continuous distribution:

.. math::

    \mu = \int_{Supp(\cD)} f(x, \theta) p(\theta) d \theta

    M_{f,\cD}(x) = \int_{Supp(\cD)} f(x, \theta)^2 p(\theta) d \theta - \mu^2

In case of a discrete distribution:

.. math::

    \mu = \sum_{\theta_i \in Supp(\cD)} f(x, \theta_i) p_i

    M_{f, \cD}(x) = \sum_{\theta_i \in Supp(\cD)} f(x, \theta_i)^2 p_i - \mu^2


Parameters
----------
function : :py:class:`openturns.Function`
    Parametric function
distribution : :py:class:`openturns.Distribution`
    Distribution of parameters

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> thetaDist = ot.Normal(2.0, 0.1)
>>> f_base = ot.SymbolicFunction(['x', 'theta'], ['x*theta'])
>>> f = ot.ParametricFunction(f_base, [1], [1.0])
>>> measure = otrobopt.VarianceMeasure(f, thetaDist)
>>> x = [1.0]
>>> print(measure(x))
[0.01])RAW"

// ---------------------------------------------------------------------
