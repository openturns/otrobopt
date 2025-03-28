%feature("docstring") OTROBOPT::JointChanceMeasure
R"RAW(Joint chance measure function.

It is built from a parametric function :math:`f` and a distribution :math:`\cD` of density function :math:`p`.

When the constraint considered is :math:`\Pset (f_k(x, \Theta) \ge 0, \forall k \in K) \geq \alpha`:

.. math::

    M_{f, \cD, \alpha}(x) = \Pset \left(\bigcap_{k \in K} \{f_k(x, \Theta) \ge 0\}\right) - \alpha

In case of a continuous distribution:

.. math::

    M_{f, \cD, \alpha}(x) = \int_{Supp(\cD)} \left( \prod_{k \in K} \mathbf{1}_{f_k(x, \theta) \ge 0} \right) p(\theta) d \theta - \alpha

In case of a discrete distribution:

.. math::

    M_{f, \cD, \alpha}(x) = \sum_{\theta_i \in Supp(\cD)}  \left( \prod_{k \in K} \mathbf{1}_{f_k(x, \theta_i) \ge 0} \right) p_i - \alpha

Parameters
----------
function : :py:class:`openturns.Function`
    Parametric function
distribution : :py:class:`openturns.Distribution`
    Distribution of parameters
operator : :py:class:`openturns.ComparisonOperator`
    Comparison operator in front of :math:`\alpha`
alpha : float in :math:`(0,1)`
    Probability level

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> thetaDist = ot.Normal(1.0, 1.0)
>>> f_base = ot.SymbolicFunction(['x', 'theta'], ['x*theta'])
>>> f = ot.ParametricFunction(f_base, [1], [1.0])
>>> measure = otrobopt.JointChanceMeasure(f, thetaDist, ot.GreaterOrEqual(), 0.95)
>>> x = [1.0]
>>> print(measure(x))
[-0.108655])RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::JointChanceMeasure::setAlpha
"Probability level accessor.

Parameters
----------
alpha : float in :math:`(0,1)`
    Probability level"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::JointChanceMeasure::getAlpha
"Probability level accessor.

Returns
-------
alpha : float in :math:`(0,1)`
    Probability level"
