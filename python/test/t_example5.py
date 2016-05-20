#!/usr/bin/env python

from __future__ import print_function
import openturns as ot
import otrobopt

thetaDist = ot.Normal(2.0, 0.1)
f_base = ot.NumericalMathFunction(['x', 'theta'], ['y'], ['x*theta'])
f = ot.NumericalMathFunction(f_base, [1], thetaDist.getMean())
measure = otrobopt.MeanMeasure(f, thetaDist)
print ('measure([1.0])=', measure([1.0]))
print ('nb evals=', f_base.getEvaluationCallsNumber())
