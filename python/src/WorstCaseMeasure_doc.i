%feature("docstring") OTROBOPT::WorstCaseMeasure
R"RAW(Worst case measure function.

It is built from a parametric function :math:`f` and a distribution :math:`\cD` of density function :math:`p`.

In case of a continuous distribution:

.. math::

    M_{f, \cD}(x) = \inf_{\theta \in Supp(\cD)} f(x, \theta)

In case of a discrete distribution:

.. math::

    M_{f, \cD}(x) = \inf_{\theta_i \in Supp(\cD)} f(x, \theta_i)


Parameters
----------
function : :py:class:`openturns.Function`
    Parametric function
distribution : :py:class:`openturns.Distribution`
    Distribution of parameters
isMinimization : bool (optional, defaults to `True`)
    Whether the worst case is the minimization of the criteria

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> thetaDist = ot.Uniform(-1.0, 4.0)
>>> f_base = ot.SymbolicFunction(['x', 'theta'], ['x*theta'])
>>> f = ot.ParametricFunction(f_base, [1], [1.0])
>>> measure = otrobopt.WorstCaseMeasure(f, thetaDist, False)
>>> x = [1.0]
>>> print(measure(x))
[4])RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::WorstCaseMeasure::setMinimization
"Minimization flag accessor.

Parameters
----------
minimization : bool
    Whether the problem is a minimization."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::WorstCaseMeasure::isMinimization
"Minimization flag accessor.

Returns
-------
minimization : bool
    Whether the problem is a minimization."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::WorstCaseMeasure::setOptimizationAlgorithm
"Optimization solver accessor.

Parameters
----------
solver : :py:class:`openturns.OptimizationAlgorithm`
    Optimization solver"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::WorstCaseMeasure::getOptimizationAlgorithm
"Optimization solver accessor.

Returns
-------
solver : :py:class:`openturns.OptimizationAlgorithm`
    Optimization solver"
