#!/usr/bin/env python

from __future__ import print_function
import openturns as ot
import openturns.testing
import otrobopt

#ot.Log.Show(ot.Log.Info)


calJ = ot.NumericalMathFunction(['x0', 'x1', 'theta'], ['J'], ['sqrt(x0) * sqrt(x1) * theta'])
g = ot.NumericalMathFunction(['x0', 'x1'], ['g'], ['-(2*x1 + 4*x0 -120)'])
J = ot.NumericalMathFunction(calJ, [2], [1.0])

dim = J.getInputDimension()


solver = ot.Cobyla()
solver.setMaximumIterationNumber(1000)

thetaDist = ot.Normal(1.0, 3.0)
robustnessMeasure = otrobopt.MeanMeasure(thetaDist, J)
problem = otrobopt.RobustOptimizationProblem()
problem.setMinimization(False)
problem.setRobustnessMeasure(robustnessMeasure)
problem.setInequalityConstraint(g)
bounds = ot.Interval([-50.0] * dim, [50.0] * dim)
problem.setBounds(bounds)


algo = otrobopt.SequentialMonteCarloRobustAlgorithm(problem, solver)
algo.setMaximumIterationNumber(10)
algo.setMaximumAbsoluteError(1e-3)
algo.setInitialSamplingSize(10)
algo.setInitialSearch(100)
algo.run()
result = algo.getResult()
#print ('x*=', result.getOptimalPoint())
openturns.testing.assert_almost_equal(result.getOptimalPoint(), [15.0, 30.0], 1e-4)
print('J(x*)=', result.getOptimalValue()[:1], 'iteration=', result.getIterationNumber())

