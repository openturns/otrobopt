from __future__ import print_function

import openturns as ot
import otrobopt
full_fun = ot.SymbolicFunction(['x', 'theta'], ['theta*x'])
param_fun = ot.ParametricFunction(full_fun, [1], [1.0])

# Normal distribution associated to parameter theta
dist_theta = ot.Normal(2.0, 1.0)

# Definition of "analytical" mean measure associated to the parametric function
mean_measure = otrobopt.MeanMeasure(param_fun, dist_theta)

mean_measure_f = otrobopt.MeasureFunction(mean_measure)

print(mean_measure_f)

print(mean_measure_f.getInputDimension())
print(mean_measure_f.getOutputDimension())
x = [2.0]
print(mean_measure_f(x))
print(mean_measure_f([x] * 10))
print(mean_measure_f.gradient(x))

# Graphical comparison of both "analytical" and "discretized" mean measures
# x_min = -2.
# x_max =  2.

# graph = ot.Graph()
# graph.setGrid(True)
# graph.setLegendPosition("topright")
# graph.add(mean_measure.draw(x_min, x_max))
# graph.add(discretized_mean_measure.draw(x_min, x_max))
# graph.setLegends(['mean', 'discretized_mean'])
# ot.Show(graph)
