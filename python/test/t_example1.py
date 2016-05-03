#!/usr/bin/env python

from __future__ import print_function
import openturns as ot
import otrobopt

#ot.Log.Show(ot.Log.Info)


calJ = ot.NumericalMathFunction(['x0', 'x1', 'theta'], ['J'], ['(x0-2)^2 + 2*x1^2 - 4*x1 + theta'])
calG = ot.NumericalMathFunction(['x0', 'x1', 'theta'], ['g'], ['-(-x0 + 4*x1 + theta - 3)'])
J = ot.NumericalMathFunction(calJ, [2], [2.0])
g = ot.NumericalMathFunction(calG, [2], [2.0])

dim = J.getInputDimension()


solver = ot.Cobyla()
solver.setMaximumIterationNumber(1000)

distributionXi = ot.Uniform(1.0, 3.0)
robustnessMeasure = otrobopt.MeanMeasure(distributionXi, J)
reliabilityMeasure = otrobopt.JointChanceMeasure(distributionXi, g, ot.Greater(), 0.9)
problem = otrobopt.RobustOptimizationProblem()
problem.setRobustnessMeasure(robustnessMeasure)
problem.setReliabilityMeasure(reliabilityMeasure)
bounds = ot.Interval([-10.0] * dim, [10.0] * dim)
problem.setBounds(bounds)


algo = otrobopt.SequentialMonteCarloRobustAlgorithm(problem, solver)
algo.setMaximumIterationNumber(10)
algo.setMaximumAbsoluteError(1e-3)
algo.setInitialSamplingSize(10)
algo.setInitialSearch(100)
algo.run()
result = algo.getResult()
print ('x*=', result.getOptimalPoint(), 'J(x*)=', result.getOptimalValue()[:1], 'iteration=', result.getIterationNumber())

