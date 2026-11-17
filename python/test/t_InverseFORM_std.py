#! /usr/bin/env python

import openturns as ot
import openturns.testing as ott
import otrobopt

ot.Log.Show(ot.Log.INFO)

# beam model, what else
full = ot.SymbolicFunction(["E", "F", "L", "b", "h"], ["F*L^3/(48.*E*b*h^3/12.)"])
g = ot.ParametricFunction(full, [2], [5.0])  # freeze L=5.0

coll = []
coll.append(ot.LogNormalMuSigmaOverMu(30000.0, 0.12, 0.0).getDistribution())  # E
coll.append(ot.LogNormalMuSigmaOverMu(0.1, 0.2, 0.0).getDistribution())  # F
coll.append(ot.LogNormalMuSigmaOverMu(0.2, 0.05, 0.0).getDistribution())  # b
coll.append(ot.LogNormalMuSigmaOverMu(0.4, 0.05, 0.0).getDistribution())  # h

distribution = ot.JointDistribution(coll)

x0 = [coll[i].computeQuantile(0.5)[0] for i in range(len(coll))]

vect = ot.RandomVector(distribution)

output = ot.CompositeRandomVector(g, vect)

event = ot.ThresholdEvent(output, ot.Greater(), 0.02)

algo = otrobopt.InverseFORM(event, "L", x0)
algo.run()
result = algo.getResult()
ott.assert_almost_equal(result.getParameter(), [5.44343], 1e-3, 1e-3)

# The analytic parameter gradient is multiplied by the operator sign, so
# swapping the sign of the limit-state function and the operator must give the
# same problem: -g < -0.02 is equivalent to g > 0.02.
signFlipped = ot.SymbolicFunction(["E", "F", "L", "b", "h"], ["-F*L^3/(48.*E*b*h^3/12.)"])
signFlippedG = ot.ParametricFunction(signFlipped, [2], [5.0])
signFlippedOutput = ot.CompositeRandomVector(signFlippedG, vect)
signFlippedEvent = ot.ThresholdEvent(signFlippedOutput, ot.Less(), -0.02)

algo = otrobopt.InverseFORM(signFlippedEvent, "L", x0)
algo.run()
ott.assert_almost_equal(algo.getResult().getParameter(), [5.44343], 1e-3, 1e-3)
