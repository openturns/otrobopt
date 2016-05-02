#!/usr/bin/env python

from __future__ import print_function
import openturns as ot
import otrobopt

#ot.Log.Show(ot.Log.Info)

# This is calligraphic J, the non-robust objective function
calJ = ot.NumericalMathFunction(['x', 'y', 'xsi'], ['J'], ['(x-2)^2 + 2*y^2 - 4*y + xsi'])

# This is calligraphic G, the non-robust inequality constraints function
calG = ot.NumericalMathFunction(['x', 'y', 'xsi'], ['g'], ['-(-x + 4*y + xsi - 3)'])

# This is capital J: J(x,xi) = calJ(x+xi), the parametric objective function
J = ot.NumericalMathFunction(calJ, [2], [2.0])

# This is g, the parametric constraints
g = ot.NumericalMathFunction(calG, [2], [2.0])

dim = J.getInputDimension()

bounds = ot.Interval([-10.0] * dim, [10.0] * dim)
solver = ot.Cobyla()
solver.setMaximumIterationNumber(100)

distributionXi = ot.Uniform(1.0, 3.0)
robustnessMeasure = otrobopt.MeanMeasure(distributionXi, J)
reliabilityMeasure = otrobopt.JointChanceMeasure(distributionXi, g, ot.Greater(), 0.9)
problem = otrobopt.RobustOptimizationProblem()
problem.setRobustnessMeasure(robustnessMeasure)
problem.setReliabilityMeasure(reliabilityMeasure)
problem.setBounds(bounds)


#ot.ResourceMap.SetAsNumericalScalar('SequentialMonteCarloRobustAlgorithm-ConvergenceFactor', 1e-5)
algo = otrobopt.SequentialMonteCarloRobustAlgorithm(problem, solver)
algo.setMaximumIterationNumber(10)
algo.setMaximumAbsoluteError(1e-3)
algo.setInitialSamplingSize(10) # size of initial xsi discretization, x2 at every iteration
algo.setInitialSearch(100) # number of multi-start tries, uniform law using bounds
algo.run()
result = algo.getResult()
print ('x*=', result.getOptimalPoint(), 'J(x*)=', result.getOptimalValue(), 'iteration=', result.getIterationNumber())
print ('g(x*)=', reliabilityMeasure(result.getOptimalPoint()))

