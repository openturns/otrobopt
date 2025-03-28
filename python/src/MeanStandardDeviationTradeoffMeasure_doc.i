%feature("docstring") OTROBOPT::MeanStandardDeviationTradeoffMeasure
R"RAW(Mean/variance tradeoff measure function.


It is built from a parametric function :math:`f` and a distribution :math:`\cD` of density function :math:`p`.

In case of a continuous distribution:

.. math::

    \mu_k = \int_{Supp(\cD)} f_k(x, \theta) p(\theta) d \theta

    Var_k = \int_{Supp(\cD)} f_k(x, \theta)^2 p(\theta) d \theta - \mu^2

    M_{f_k, \cD, \alpha_k}(x) = (1-\alpha_k) \mu_k + \alpha_k \sqrt{Var_k}

In case of a discrete distribution:

.. math::

    \mu_k = \sum_{\theta_i \in Supp(\cD)} f_k(x, \theta_i) p_i

    Var_k = \sum_{\theta_i \in Supp(\cD)} f_k(x, \theta_i)^2 p_i - {\mu_k}^2

    M_{f_k, \cD, \alpha_k}(x) = (1-\alpha_k) \mu_k + \alpha_k \sqrt{Var_k}


Parameters
----------
function : :py:class:`openturns.Function`
    Parametric function
distribution : :py:class:`openturns.Distribution`
    Distribution of parameters
alpha : sequence of float, :math:`\alpha_k \in [0,1]`
    Tradeoff coefficient

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> thetaDist = ot.Normal(2.0, 0.1)
>>> f_base = ot.SymbolicFunction(['x', 'theta'], ['x*theta'])
>>> f = ot.ParametricFunction(f_base, [1], [1.0])
>>> measure = otrobopt.MeanStandardDeviationTradeoffMeasure(f, thetaDist, [0.4])
>>> x = [1.0]
>>> print(measure(x))
[1.24])RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::MeanStandardDeviationTradeoffMeasure::setAlpha
"Tradeoff coefficient accessor.

Parameters
----------
alpha : sequence of  float in :math:`(0,1)`
    Tradeoff coefficient"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::MeanStandardDeviationTradeoffMeasure::getAlpha
"Tradeoff coefficient accessor.

Returns
-------
alpha : sequence of float in :math:`(0,1)`
    Tradeoff coefficient"
