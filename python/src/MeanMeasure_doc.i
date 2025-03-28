%feature("docstring") OTROBOPT::MeanMeasure
R"RAW(Mean measure function.

It is built from a parametric function :math:`f` and a distribution :math:`\cD` of density function :math:`p`.


In case of a continuous distribution:

.. math::

    M_{f, \cD}(x) = \int_{Supp(\cD)} f(x, \theta) p(\theta) d \theta

In case of a discrete distribution:

.. math::

    M_{f, \cD}(x) = \sum_{\theta_i \in Supp(\cD)} f(x, \theta_i) p_i


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
>>> measure = otrobopt.MeanMeasure(f, thetaDist)
>>> x = [1.0]
>>> print(measure(x))
[2])RAW"

