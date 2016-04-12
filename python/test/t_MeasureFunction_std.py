#! /usr/bin/env python

from __future__ import print_function
import openturns as ot
import otrobopt

thetaDist = ot.Normal(2.0, 0.1)
f = ot.NumericalMathFunction(['x1', 'p1'], ['y1'], ['x1*p1'])
parametric = ot.NumericalMathFunction(f, [1], [0., 1.])

x = [1.0]

measures = [otrobopt.MeanMeasure(thetaDist, parametric),
            otrobopt.VarianceMeasure(thetaDist, parametric),
            otrobopt.WorstCaseMeasure(thetaDist, parametric),
            otrobopt.WorstCaseMeasure(thetaDist, parametric, False),
            otrobopt.JointChanceMeasure(thetaDist, parametric, 0.95),
            otrobopt.IndividualChanceMeasure(thetaDist, parametric, 0.95),
            otrobopt.MeanStandardDeviationTradeoffMeasure(thetaDist, parametric, 0.4),
            otrobopt.QuantileMeasure(thetaDist, parametric, 0.8)
            ]
aggregated = otrobopt.AggregatedMeasure(measures)
measures.append(aggregated)

for measure in measures:
    print(measure, '(continuous)', measure(x))
    N = 10000
    experiment = ot.LHSExperiment(thetaDist, N)
    factory = otrobopt.MeasureFactory(measure, experiment)
    discretizedMeasure = factory.build()
    print(discretizedMeasure, '(discretized)', discretizedMeasure(x))
