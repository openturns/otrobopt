#!/usr/bin/env python

from __future__ import print_function
import openturns as ot
import otrobopt

#ot.Log.Show(ot.Log.Info)


calJ = ot.NumericalMathFunction(['x', 'theta'], ['J'], ['x^3 - 3*x + theta'])
calG = ot.NumericalMathFunction(['x', 'theta'], ['g'], ['-(x + theta - 2)'])
J = ot.NumericalMathFunction(calJ, [1], [0.5])
g = ot.NumericalMathFunction(calG, [1], [0.5])

dim = J.getInputDimension()


solver = ot.Cobyla()
solver.setMaximumIterationNumber(1000)

distributionXi = ot.Exponential(2.0)
robustnessMeasure = otrobopt.MeanMeasure(distributionXi, J)
reliabilityMeasure = otrobopt.JointChanceMeasure(distributionXi, g, ot.Greater(), 0.9)
problem = otrobopt.RobustOptimizationProblem()
problem.setMinimization(False)
problem.setRobustnessMeasure(robustnessMeasure)
problem.setReliabilityMeasure(reliabilityMeasure)
bounds = ot.Interval([-20.0] * dim, [2.0] * dim)
problem.setBounds(bounds)


algo = otrobopt.SequentialMonteCarloRobustAlgorithm(problem, solver)
algo.setMaximumIterationNumber(10)
algo.setMaximumAbsoluteError(1e-3)
algo.setInitialSamplingSize(10)
algo.setInitialSearch(100)
algo.run()
result = algo.getResult()
print ('x*=', result.getOptimalPoint(), 'J(x*)=', result.getOptimalValue()[:1], 'iteration=', result.getIterationNumber())
