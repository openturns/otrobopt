import openturns as ot
import openturns.testing as ott
import otrobopt

full_fun = ot.SymbolicFunction(["x", "theta"], ["theta*x"])
param_fun = ot.ParametricFunction(full_fun, [1], [1.0])

# Normal distribution associated to parameter theta
dist_theta = ot.Normal(2.0, 1.0)

# Definition of "analytical" mean measure associated to the parametric function
mean_measure = otrobopt.MeanMeasure(param_fun, dist_theta)

mean_measure_f = otrobopt.MeasureFunction(mean_measure)

ott.assert_almost_equal(mean_measure_f.getInputDimension(), 1)
ott.assert_almost_equal(mean_measure_f.getOutputDimension(), 1)
x = [2.0]
ott.assert_almost_equal(mean_measure_f(x), [4.0])
ott.assert_almost_equal(mean_measure_f([x] * 10), [[4.0]] * 10)
ott.assert_almost_equal(mean_measure_f.gradient(x), ot.Matrix([[2.0]]))
