#! /usr/bin/env python

import openturns as ot
import openturns.testing as ott
import otrobopt

ot.Log.Show(ot.Log.INFO)

# sphere under pressure
full = ot.SymbolicFunction(["R", "e", "mulog_e", "p"], ["700.0-p*R/(2.*e)"])
mulog_e_0 = -4.715
g = ot.ParametricFunction(full, [2], [mulog_e_0])

mulog_eDist = ot.Dirac(mulog_e_0)
mulog_eDist.setDescription(["mulog_e"])
eColl = [mulog_eDist, ot.Dirac(0.1), ot.Dirac(0.0)]
eParams = ot.JointDistribution(eColl)
try:
    eDist = ot.DeconditionedDistribution(ot.LogNormal(mulog_e_0, 0.1, 0.), eParams)
except AttributeError:
    eDist = ot.ConditionalDistribution(ot.LogNormal(mulog_e_0, 0.1, 0.), eParams)

coll = [ot.Beta(0.117284, 0.117284, 2.9, 3.1),
        eDist,
        ot.WeibullMin(3.16471, 9.21097, 0.0)]

distribution = ot.JointDistribution(coll)
median = [distribution.getMarginal(i).computeQuantile(0.5)[0] for i in range(3)]

vect = ot.RandomVector(distribution)

output = ot.CompositeRandomVector(g, vect)

event = ot.ThresholdEvent(output, ot.Less(), 0.0)

algo = otrobopt.InverseFORM(event, 'mulog_e', median)
algo.run()
result = algo.getResult()
print(result)
ott.assert_almost_equal(result.getParameter(), [-4.69056])
