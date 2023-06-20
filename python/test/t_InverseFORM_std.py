#! /usr/bin/env python

import openturns as ot
import openturns.testing as ott
import otrobopt

ot.Log.Show(ot.Log.INFO)

# beam model, what else
full = ot.SymbolicFunction(['E', 'F', 'L', 'b', 'h'], ['F*L^3/(48.*E*b*h^3/12.)'])
g = ot.ParametricFunction(full, [2], [5.0])  # freeze L=5.0

coll = []
coll.append(ot.LogNormalMuSigmaOverMu(30000., 0.12, 0.).getDistribution())  # E
coll.append(ot.LogNormalMuSigmaOverMu(0.1, 0.2, 0.).getDistribution())  # F
coll.append(ot.LogNormalMuSigmaOverMu(0.2, 0.05, 0.).getDistribution())  # b
coll.append(ot.LogNormalMuSigmaOverMu(0.4, 0.05, 0.).getDistribution())  # h

distribution = ot.ComposedDistribution(coll)

x0 = [coll[i].computeQuantile(0.5)[0] for i in range(len(coll))]

vect = ot.RandomVector(distribution)

output = ot.CompositeRandomVector(g, vect)

event = ot.ThresholdEvent(output, ot.Greater(), 0.02)

algo = otrobopt.InverseFORM(event, 'L', x0)
algo.run()
result = algo.getResult()
print(result)
ott.assert_almost_equal(result.getParameter(), [5.44343])
