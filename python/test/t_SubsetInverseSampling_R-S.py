#! /usr/bin/env python

# This script shows the use of the module SubsetInverse on the model G=R-S,
# with R~N, S~N

import openturns as ot
import openturns.testing as ott
import otrobopt

ot.TESTPREAMBLE()


def cleanScalar(inScalar):
    if abs(inScalar) < 1.0e-10:
        inScalar = 0.0
    return inScalar


###########################################################################
# Physical model
###########################################################################

limitState = ot.SymbolicFunction(["u1", "u2"], ["u1-u2"])
dim = limitState.getInputDimension()

###########################################################################
# Probabilistic model
###########################################################################

mean = [7.0, 2.0]
sigma = [1.0] * 2

R = ot.IdentityMatrix(dim)
myDistribution = ot.Normal(mean, sigma, R)

start = myDistribution.getMean()

###########################################################################
# Limit state
###########################################################################

vect = ot.RandomVector(myDistribution)

output = ot.CompositeRandomVector(limitState, vect)

threshold = 0.0
myEvent = ot.ThresholdEvent(output, ot.Less(), threshold)

###########################################################################
# Computation Monte Carlo
###########################################################################
bs = 1

# Monte Carlo
experiment = ot.MonteCarloExperiment()
myMC = ot.ProbabilitySimulationAlgorithm(myEvent, experiment)
myMC.setMaximumOuterSampling(int(1e6) // bs)
myMC.setBlockSize(bs)
myMC.setMaximumCoefficientOfVariation(-1)
myMC.run()

###########################################################################
# Results Monte Carlo
###########################################################################

ResultMC = myMC.getResult()
PFMC = ResultMC.getProbabilityEstimate()
CVMC = ResultMC.getCoefficientOfVariation()
Variance_PF_MC = ResultMC.getVarianceEstimate()
length90MC = ResultMC.getConfidenceLength(0.90)
N_MC = ResultMC.getOuterSampling() * ResultMC.getBlockSize()

###########################################################################
# Computation SubsetSampling
###########################################################################

finalTargetProbability = 0.0002
mySS = otrobopt.SubsetInverseSampling(myEvent, finalTargetProbability)
mySS.setMaximumOuterSampling(10000 // bs)
mySS.setBlockSize(bs)
mySS.run()

###########################################################################
# Results SubsetSampling
###########################################################################

ResultSS = mySS.getResult()
PFSS = ResultSS.getProbabilityEstimate()
CVSS = ResultSS.getCoefficientOfVariation()
Variance_PF_SS = ResultSS.getVarianceEstimate()
length90SS = ResultSS.getConfidenceLength(0.90)
N_SS = ResultSS.getOuterSampling() * ResultSS.getBlockSize()
thresholdSS = mySS.getThresholdPerStep()[-1]
thLengthSS = mySS.getThresholdConfidenceLength(0.90)

###########################################################################

ott.assert_almost_equal(len(mySS.getThresholdPerStep()), 4)
