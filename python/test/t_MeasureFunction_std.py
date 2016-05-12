from __future__ import print_function

import openturns as ot
import otrobopt
full_fun     = ot.NumericalMathFunction(["x", "theta"], ["theta*x"])
param_fun    = ot.NumericalMathFunction(full_fun, [1], [1.0])

# Normal distribution associated to parameter theta
dist_theta   = ot.Normal(2.0, 1.0)

# Definition of "analytical" mean measure associated to the parametric function
mean_measure = otrobopt.MeanMeasure(param_fun, dist_theta)

# Definition of "discretized" mean measure
N = 10
experiment = ot.LHSExperiment(dist_theta, N)
factory    = otrobopt.MeasureFactory(mean_measure, experiment)
discretized_mean_measure_evaluation = factory.build()

discretized_mean_measure = otrobopt.MeasureFunction(discretized_mean_measure_evaluation)

print(discretized_mean_measure)

print(discretized_mean_measure.getInputDimension())
print(discretized_mean_measure.getOutputDimension())
x = [2.0]
print(discretized_mean_measure(x))
print(discretized_mean_measure([x] * 10))
print(discretized_mean_measure.gradient(x))

## Graphical comparison of both "analytical" and "discretized" mean measures
#x_min = -2.
#x_max =  2.

#graph = ot.Graph()
#graph.setGrid(True)
#graph.setLegendPosition("topright")
#graph.add(mean_measure.draw(x_min, x_max))
#graph.add(discretized_mean_measure.draw(x_min, x_max))
#graph.setLegends(['mean', 'discretized_mean'])
#ot.Show(graph)

