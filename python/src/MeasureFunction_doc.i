%feature("docstring") OTROBOPT::MeasureFunction
"Measure function.

Parameters
----------
evaluation : :class:`~otrobopt.MeasureEvaluation`
    Measure evaluation

Examples
--------
First define a measure:

>>> import openturns as ot
>>> import otrobopt
>>> thetaDist = ot.Normal(2.0, 0.1)
>>> f = ot.SymbolicFunction(['x', 'theta'], ['x*theta'])
>>> parametric = ot.ParametricFunction(f, [1], [1.0])
>>> evaluation = otrobopt.MeanMeasure(parametric, thetaDist)
>>> function = otrobopt.MeasureFunction(evaluation)"
