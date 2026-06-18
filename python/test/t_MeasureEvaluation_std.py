#!/usr/bin/env python

import openturns as ot
import openturns.testing as ott
import otrobopt

# First test: theta of dimension 1
thetaDist = ot.Normal(2.0, 0.1)
f_base = ot.SymbolicFunction(["x", "theta"], ["x*theta"])
f = ot.ParametricFunction(f_base, [1], [1.0])

x = [1.0]

measures = [
    otrobopt.MeanMeasure(f, thetaDist),
    otrobopt.VarianceMeasure(f, thetaDist),
    otrobopt.WorstCaseMeasure(f, ot.Uniform(-1.0, 4.0)),
    otrobopt.WorstCaseMeasure(f, ot.Uniform(-1.0, 4.0), False),
    otrobopt.JointChanceMeasure(f, ot.Normal(1.0, 1.0), ot.GreaterOrEqual(), 0.95),
    otrobopt.IndividualChanceMeasure(
        f, ot.Normal(1.0, 1.0), ot.GreaterOrEqual(), [0.95]
    ),
    otrobopt.MeanStandardDeviationTradeoffMeasure(f, thetaDist, [0.8]),
    otrobopt.QuantileMeasure(f, thetaDist, 0.99),
]
aggregated = otrobopt.AggregatedMeasure(measures)
measures.append(aggregated)

for i, measure in enumerate(measures):
    expectedDim = i if isinstance(measure, otrobopt.AggregatedMeasure) else 1
    ott.assert_almost_equal(measure(x).getDimension(), expectedDim)
    N = 10000
    experiment = ot.LHSExperiment(N)
    factory = otrobopt.MeasureFactory(experiment)
    discretizedMeasure = factory.build(measure)
    ott.assert_almost_equal(discretizedMeasure(x).getDimension(), expectedDim)
    N = 4
    experiment = ot.GaussProductExperiment([N])
    factory = otrobopt.MeasureFactory(experiment)
    discretizedMeasure = factory.build(measure)
    ott.assert_almost_equal(discretizedMeasure(x).getDimension(), expectedDim)

# Test known continuous values for dimension 1
ott.assert_almost_equal(measures[0](x), [2.0])  # MeanMeasure
ott.assert_almost_equal(measures[1](x), [0.01])  # VarianceMeasure
ott.assert_almost_equal(measures[2](x), [-1.0])  # WorstCase min
ott.assert_almost_equal(measures[3](x), [4.0])  # WorstCase max
ott.assert_almost_equal(measures[6](x), [0.48])  # MeanStdTradeoff

# Second test: theta of dimension 2
thetaDist = ot.Normal([2.0] * 2, [0.1] * 2, ot.IdentityMatrix(2))
f_base = ot.SymbolicFunction(["x", "theta0", "theta1"], ["x*theta0+theta1"])
f = ot.ParametricFunction(f_base, [1, 2], thetaDist.getMean())

x = [1.0]

measures = [
    otrobopt.MeanMeasure(f, thetaDist),
    otrobopt.VarianceMeasure(f, thetaDist),
    otrobopt.WorstCaseMeasure(f, ot.JointDistribution([ot.Uniform(-1.0, 4.0)] * 2)),
    otrobopt.WorstCaseMeasure(
        f, ot.JointDistribution([ot.Uniform(-1.0, 4.0)] * 2), False
    ),
    otrobopt.JointChanceMeasure(f, thetaDist, ot.GreaterOrEqual(), 0.95),
    otrobopt.IndividualChanceMeasure(f, thetaDist, ot.GreaterOrEqual(), [0.5]),
    otrobopt.MeanStandardDeviationTradeoffMeasure(f, thetaDist, [0.8]),
]
aggregated = otrobopt.AggregatedMeasure(measures)
measures.append(aggregated)

for i, measure in enumerate(measures):
    expectedDim = i if isinstance(measure, otrobopt.AggregatedMeasure) else 1
    ott.assert_almost_equal(measure(x).getDimension(), expectedDim)
    N = 10000
    experiment = ot.LHSExperiment(N)
    factory = otrobopt.MeasureFactory(experiment)
    discretizedMeasure = factory.build(measure)
    ott.assert_almost_equal(discretizedMeasure(x).getDimension(), expectedDim)
    N = 4
    experiment = ot.GaussProductExperiment(measure.getDistribution(), [N] * 2)
    factory = otrobopt.MeasureFactory(experiment)
    discretizedMeasure = factory.build(measure)
    ott.assert_almost_equal(discretizedMeasure(x).getDimension(), expectedDim)

# Test known continuous values for dimension 2
ott.assert_almost_equal(measures[0](x), [4.0])  # MeanMeasure: x*2.0+2.0 = 4.0
ott.assert_almost_equal(measures[1](x), [0.02])  # VarianceMeasure
ott.assert_almost_equal(measures[2](x), [-2.0])  # WorstCase min
ott.assert_almost_equal(measures[3](x), [8.0])  # WorstCase max
