#! /usr/bin/env python

from __future__ import print_function
import openturns as ot
import otrobopt

a = otrobopt.MeasureFunction()
print(a)

thetaDist = ot.Normal(2, 0.1)
f = ot.NumericalMathFunction(['x1', 'p1'], ['y1'], ['x1*p1'])
parametric = ot.NumericalMathFunction(f, [1], [0., 1.])

x = [1.0]

meanMeasure = otrobopt.MeanMeasure(thetaDist, parametric)
print(meanMeasure, meanMeasure(x))

varianceMeasure = otrobopt.VarianceMeasure(thetaDist, parametric)
print(varianceMeasure, varianceMeasure(x))

