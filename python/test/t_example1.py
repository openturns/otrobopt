#!/usr/bin/env python

from __future__ import print_function
import openturns as ot
import otrobopt

#ot.Log.Show(ot.Log.Info)


calJ = ot.SymbolicFunction(['x0', 'x1', 'theta'], ['(x0-2)^2 + 2*x1^2 - 4*x1 + theta'])
calG = ot.SymbolicFunction(['x0', 'x1', 'theta'], ['-(-x0 + 4*x1 + theta - 3)'])
J = ot.ParametricFunction(calJ, [2], [2.0])
g = ot.ParametricFunction(calG, [2], [2.0])

dim = J.getInputDimension()


solver = ot.Cobyla()
solver.setMaximumIterationNumber(1000)

thetaDist = ot.Uniform(1.0, 3.0)
robustnessMeasure = otrobopt.MeanMeasure(J, thetaDist)
reliabilityMeasure = otrobopt.JointChanceMeasure(g, thetaDist, ot.Greater(), 0.9)
problem = otrobopt.RobustOptimizationProblem(robustnessMeasure, reliabilityMeasure)
bounds = ot.Interval([-10.0] * dim, [10.0] * dim)
problem.setBounds(bounds)


algo = otrobopt.SequentialMonteCarloRobustAlgorithm(problem, solver)
algo.setMaximumIterationNumber(10)
algo.setMaximumAbsoluteError(1e-3)
algo.setInitialSamplingSize(10)
algo.setInitialSearch(100)
algo.run()
result = algo.getResult()
print ('x*=', result.getOptimalPoint(), 'J(x*)=', result.getOptimalValue(), 'iteration=', result.getIterationNumber())

