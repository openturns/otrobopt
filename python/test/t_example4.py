#!/usr/bin/env python

from __future__ import print_function
import openturns as ot
import otrobopt

#ot.Log.Show(ot.Log.Info)


calJ = ot.SymbolicFunction(['x', 'theta'], ['cos(x) * sin(theta)'])
calG = ot.SymbolicFunction(['x', 'theta'], ['-(-2 - x + theta)', '-(x - 4)'])
J = ot.ParametricFunction(calJ, [1], [1.0])
g = ot.ParametricFunction(calG, [1], [1.0])

dim = J.getInputDimension()


solver = ot.Cobyla()
solver.setMaximumIterationNumber(1000)
solver.setStartingPoint([0.0] * dim)

thetaDist = ot.Uniform(0.0, 2.0)
robustnessMeasure = otrobopt.MeanMeasure(J, thetaDist)
reliabilityMeasure = otrobopt.JointChanceMeasure(g, thetaDist, ot.Greater(), 0.9)
problem = otrobopt.RobustOptimizationProblem(robustnessMeasure, reliabilityMeasure)

algo = otrobopt.SequentialMonteCarloRobustAlgorithm(problem, solver)
algo.setMaximumIterationNumber(10)
algo.setMaximumAbsoluteError(1e-3)
algo.setInitialSamplingSize(10)
algo.run()
result = algo.getResult()
print ('x*=', result.getOptimalPoint(), 'J(x*)=', result.getOptimalValue(), 'iteration=', result.getIterationNumber())

