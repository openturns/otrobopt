%feature("docstring") OTROBOPT::IndividualChanceMeasure
R"RAW(Individual chance measure function.

It is built from a parametric function :math:`f` and a distribution :math:`\cD` of density function :math:`p`.

When the constraint considered is :math:`\Pset (f_k(x, \Theta) \ge 0) \geq \alpha_k, \forall k \in K`:

.. math::

    M_{f_k, \cD, \alpha_k}(x) = \Pset (f_k(x, \theta) \ge 0) - \alpha_k

In case of a continuous distribution:

.. math::

    M_{f_k, \cD, \alpha_k}(x) = \int_{Supp(\cD)} \mathbf{1}_{f_k(x, \theta) \ge 0} p(\theta) d \theta - \alpha_k

In case of a discrete distribution:

.. math::

    M_{f_k, \cD, \alpha_k}(x) = \sum_{\theta_i \in Supp(\cD)} \mathbf{1}_{f_k(x, \theta_i) \ge 0} p_i - \alpha_k


Parameters
----------
function : :py:class:`openturns.Function`
    Parametric function
distribution : :py:class:`openturns.Distribution`
    Distribution of parameters
operator : :py:class:`openturns.ComparisonOperator`
    Comparison operator in front of :math:`\alpha_k`
alpha : sequence of float
    Probability levels

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> thetaDist = ot.Normal(1.0, 1.0)
>>> f_base = ot.SymbolicFunction(['x', 'theta'], ['x*theta'])
>>> f = ot.ParametricFunction(f_base, [1], [1.0])
>>> measure = otrobopt.IndividualChanceMeasure(f, thetaDist, ot.GreaterOrEqual(), [0.95])
>>> x = [1.0]
>>> print(measure(x))
[-0.108655])RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::IndividualChanceMeasure::setAlpha
"Probability levels accessor.

Parameters
----------
alpha : sequence of float in :math:`(0,1)`
    Probability levels"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::IndividualChanceMeasure::getAlpha
"Probability levels accessor.

Returns
-------
alpha : sequence of float in :math:`(0,1)`
    Probability levels"
