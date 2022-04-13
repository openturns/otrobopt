#!/usr/bin/env python

import openturns as ot
import otrobopt
import os

fileName = 'myStudy.xml'

# Create a Study Object
myStudy = ot.Study()
myStudy.setStorageManager(ot.XMLStorageManager(fileName))

thetaDist = ot.Normal(2.0, 0.1)
f_base = ot.SymbolicFunction(['x', 'theta'], ['x*theta'])
f = ot.ParametricFunction(f_base, [1], [1.0])

measures = [otrobopt.MeanMeasure(f, thetaDist),
            otrobopt.VarianceMeasure(f, thetaDist),
            otrobopt.WorstCaseMeasure(f, ot.Uniform(-1.0, 4.0), False),
            otrobopt.JointChanceMeasure(
                f, ot.Normal(1.0, 1.0), ot.GreaterOrEqual(), 0.95),
            otrobopt.IndividualChanceMeasure(
                f, ot.Normal(1.0, 1.0), ot.GreaterOrEqual(), [0.95]),
            otrobopt.MeanStandardDeviationTradeoffMeasure(f, thetaDist, [0.8]),
            otrobopt.QuantileMeasure(f, thetaDist, 0.99)
            ]
aggregated = otrobopt.AggregatedMeasure(measures)
measures.append(aggregated)

for measure in measures:
    myStudy.add('measure' + measure.__class__.__name__, measure)

measure2 = otrobopt.MeanMeasure(f, thetaDist)
measureFunction = otrobopt.MeasureFunction(measure2)
myStudy.add('measureFunction', measureFunction)

calJ = ot.SymbolicFunction(
    ['x1', 'x2'], ['15.0 * (x1^2 + x2^2) - 100.0 * exp(-5. * ((x1 + 1.6)^2+(x2 + 1.6)^2))'])
# This is calligraphic G, the non-robust inequality constraints function
calG = ot.SymbolicFunction(
    ['x1', 'x2'], ['(x1 - 0.5)^2 + x2^2 - 4.0', '(x1 + 0.5)^2 + x2^2 - 4.0'])
# This is the perturbation function
noise = ot.SymbolicFunction(
    ['x1', 'x2', 'xi1', 'xi2'], ['x1 + xi1', 'x2 + xi2'])
# This is capital J: J(x,xi) = calJ(x+xi), the parametric objective function
JFull = ot.ComposedFunction(calJ, noise)
J = ot.ParametricFunction(JFull, [2, 3], [0.0] * 2)
# This is g, the parametric constraints
gFull = ot.ComposedFunction(calG, noise)
g = ot.ParametricFunction(gFull, [2, 3], [0.0] * 2)
bounds = ot.Interval([-3.0] * 2, [3.0] * 2)
sigma_xi = 0.4
thetaDist = ot.Normal([0.0] * 2, [sigma_xi] * 2, ot.IdentityMatrix(2))
robustnessMeasure = otrobopt.MeanMeasure(J, thetaDist)
reliabilityMeasure = otrobopt.JointChanceMeasure(g, thetaDist, ot.Less(), 0.9)
problem = otrobopt.RobustOptimizationProblem(
    robustnessMeasure, reliabilityMeasure)
problem.setBounds(bounds)
myStudy.add('problem', problem)

solver = ot.NLopt('LD_SLSQP')
solver.setMaximumIterationNumber(100)
algo = otrobopt.SequentialMonteCarloRobustAlgorithm(problem, solver)
algo.setMaximumIterationNumber(11)
algo.setMaximumAbsoluteError(1e-6)
algo.setInitialSamplingSize(
    2)  # size of initial xsi discretization, x2 at every iteration
algo.setInitialSearch(1000)
                      # number of multi-start tries, uniform law using bounds
myStudy.add('algo', algo)

myStudy.save()

# Create a new Study Object
myStudy = ot.Study()
myStudy.setStorageManager(ot.XMLStorageManager(fileName))

myStudy.load()

for measureCName in list(map(lambda x: x.__class__.__name__, measures)):
    measure = eval('otrobopt.' + measureCName + '()')
    myStudy.fillObject('measure' + measureCName, measure)
    print('measure= ', measure)

measureFunction = otrobopt.MeasureFunction()
myStudy.fillObject('measureFunction', measureFunction)
print('measureFunction= ', measureFunction)

problem = otrobopt.RobustOptimizationProblem()
myStudy.fillObject('problem', problem)
print('problem= ', problem)

algo = otrobopt.SequentialMonteCarloRobustAlgorithm()
myStudy.fillObject('algo', algo)
print('algo= ', algo)

# cleanup
os.remove(fileName)
