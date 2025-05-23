%feature("docstring") OTROBOPT::MeasureFactory
R"RAW(Discretize a measure function.

It consists in replacing the distribution :math:`\cD` of the measure by a discrete
approximation.

.. math::

    \cD \rightarrow \sum_{i=1}^N \omega_i \delta_{\theta_i}


Where :math:`\delta_{\theta_i}` is the Dirac measure at :math:`\theta_i`.

Parameters
----------
experiment : :py:class:`openturns.WeightedExperiment`
    Parameters design of experiment

Examples
--------
First define a measure:

>>> import openturns as ot
>>> import otrobopt
>>> thetaDist = ot.Normal(2.0, 0.1)
>>> f_base = ot.SymbolicFunction(['x', 'theta'], ['x*theta'])
>>> f = ot.ParametricFunction(f_base, [1], [1.0])
>>> measure = otrobopt.MeanMeasure(f, thetaDist)


Then discretize it:

>>> N = 10
>>> experiment = ot.LHSExperiment(N)
>>> factory = otrobopt.MeasureFactory(experiment)
>>> discretizedMeasure = factory.build(measure)


Discretize several measures at once:

>>> coll = [otrobopt.MeanMeasure(f, thetaDist),
...         otrobopt.VarianceMeasure(f, thetaDist)]
>>> discretizedMeasures = factory.buildCollection(coll)
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::MeasureFactory::build
"Discretize a measure.

Parameters
----------
measure : :class:`~otrobopt.MeasureEvaluation`
    Measure

Returns
-------
measure : :class:`~otrobopt.MeasureEvaluation`
    Discretized measure"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::MeasureFactory::buildCollection
"Discretize several measures.

Parameters
----------
collection : sequence of :class:`~otrobopt.MeasureEvaluation`
    The measures to discretize.

Returns
-------
measures : sequence of :class:`~otrobopt.MeasureEvaluation`
    Discretized measures"
