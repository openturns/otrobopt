#!/usr/bin/env python

from __future__ import print_function
import openturns as ot
import otrobopt

#ot.Log.Show(ot.Log.Info)

# This is calligraphic J, the non-robust objective function
calJ = ot.SymbolicFunction(['x1', 'x2'], ['15.0 * (x1^2 + x2^2) - 100.0 * exp(-5. * ((x1 + 1.6)^2+(x2 + 1.6)^2))'])

# This is calligraphic G, the non-robust inequality constraints function
calG = ot.SymbolicFunction(['x1', 'x2'], ['(x1 - 0.5)^2 + x2^2 - 4.0', '(x1 + 0.5)^2 + x2^2 - 4.0'])

# This is the perturbation function
noise = ot.SymbolicFunction(['x1', 'x2', 'xi1', 'xi2'], ['x1 + xi1', 'x2 + xi2'])

# This is capital J: J(x,xi) = calJ(x+xi), the parametric objective function
JFull = ot.ComposedFunction(calJ, noise)
J = ot.ParametricFunction(JFull, [2, 3], [0.0] * 2)

# This is g, the parametric constraints
gFull = ot.ComposedFunction(calG, noise)
g = ot.ParametricFunction(gFull, [2, 3], [0.0] * 2)


bounds = ot.Interval([-3.0] * 2, [3.0] * 2)
solver = ot.NLopt('LD_SLSQP')
solver.setMaximumIterationNumber(100)

for sigma_xi in [0.1, 0.2, 0.3, 0.4, 0.5]:

    thetaDist = ot.Normal([0.0] * 2, [sigma_xi] * 2, ot.IdentityMatrix(2))
    robustnessMeasure = otrobopt.MeanMeasure(J, thetaDist)
    reliabilityMeasure = otrobopt.JointChanceMeasure(g, thetaDist, ot.Less(), 0.9)
    problem = otrobopt.RobustOptimizationProblem(robustnessMeasure, reliabilityMeasure)
    problem.setBounds(bounds)

    algo = otrobopt.SequentialMonteCarloRobustAlgorithm(problem, solver)
    algo.setMaximumIterationNumber(11)
    algo.setMaximumAbsoluteError(1e-6)
    algo.setInitialSamplingSize(2) # size of initial xsi discretization
    algo.setSamplingSizeIncrement(ot.PythonFunction(1, 1, lambda x:[1.0*x[0]]))
    algo.setInitialSearch(1000) # number of multi-start tries, uniform law using bounds
    algo.run()
    result = algo.getResult()
    print ('x*=', result.getOptimalPoint(), 'J(x*)=', result.getOptimalValue(), 'iteration=', result.getIterationNumber())
    #print (algo.getInitialResultCollection())
    #print (algo.getInitialStartingPoints())
    #print (algo.getResultCollection())
