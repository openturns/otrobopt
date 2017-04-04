from __future__ import print_function

import openturns as ot
import otrobopt
full_fun     = ot.Function(['x', 'theta'], ['theta*x'])
param_fun    = ot.Function(full_fun, [1], [1.0])

# Normal distribution associated to parameter theta
thetaDist   = ot.Normal(2.0, 0.1)

# Definition of "analytical" mean measure associated to the parametric function
mean_measure = otrobopt.MeanMeasure(param_fun, thetaDist)
variance_measure = otrobopt.VarianceMeasure(param_fun, thetaDist)
# Definition of "discretized" mean measure
N = 10
experiment = ot.LHSExperiment(thetaDist, N)
factory    = otrobopt.MeasureFactory(experiment)
discretized_mean_measure_evaluation = factory.build(mean_measure)

coll = [mean_measure, variance_measure]
discretizedColl = factory.buildCollection(coll)
print(coll)

x = [1.0]
print(otrobopt.AggregatedMeasure(coll)(x))
