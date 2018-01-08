#! /usr/bin/env python

# This script shows the use of the module SubsetInverse on the model G=R-S,
# with R~N, S~N

from __future__ import print_function, division
from openturns import *
from math import *
from otrobopt import *

TESTPREAMBLE()

def cleanScalar(inScalar) :
    if (fabs(inScalar) < 1.e-10) :
        inScalar = 0.0
    return inScalar

###########################################################################
# initialize the random generator
###########################################################################

RandomGenerator.SetSeed(0)


###########################################################################
# Physical model
###########################################################################

limitState = SymbolicFunction(['u1', 'u2'], ['u1-u2'])
dim = limitState.getInputDimension()

###########################################################################
# Probabilistic model
###########################################################################

mean = Point(dim, 0.0)
mean[0] = 7.
mean[1] = 2.
sigma = Point(dim, 1.0)

R = IdentityMatrix(dim)
myDistribution = Normal(mean, sigma, R)

start = myDistribution.getMean()

###########################################################################
# Limit state
###########################################################################

vect = RandomVector(myDistribution)

output = RandomVector(limitState, vect)

threshold = 0.
myEvent = Event(output, ComparisonOperator(Less()), threshold)

###########################################################################
# Computation Monte Carlo
###########################################################################
bs = 1

# Monte Carlo
experiment = MonteCarloExperiment()
myMC = ProbabilitySimulationAlgorithm(myEvent, experiment)
myMC.setMaximumOuterSampling(int(1e6)// bs)
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
N_MC = ResultMC.getOuterSampling()*ResultMC.getBlockSize()

###########################################################################
# Computation SubsetSampling
###########################################################################

finalTargetProbability = 0.0002
mySS = SubsetInverseSampling(myEvent, finalTargetProbability)
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
N_SS = ResultSS.getOuterSampling()*ResultSS.getBlockSize()
thresholdSS = mySS.getThresholdPerStep()[-1]
thLengthSS = mySS.getThresholdConfidenceLength(0.90)

###########################################################################

print("")
print("*******************************************************************")
print("***************************** MONTE CARLO *************************")
print("*******************************************************************")
print("Pf estimation = %.5e" % PFMC)
print("Pf Variance estimation = %.5e" % Variance_PF_MC)
print("CoV = %.5f" % CVMC)
print("90% Confidence Interval =" , "%.5e" % length90MC)
print("CI at 90% =[", "%.5e" % (PFMC-0.5*length90MC) ,
      "; %.5e" % (PFMC+0.5*length90MC) , "]")
print("Threshold = %.5e" % threshold)
print("Limit state calls =", N_MC)
print("*******************************************************************")
print("")
print("*******************************************************************")
print("************************** SUBSET SAMPLING ************************")
print("*******************************************************************")
print("Pf estimation = %.5e" % PFSS)
print("Pf Variance estimation = %.5e" % Variance_PF_SS)
print("CoV = %.5f" % CVSS)
print("90% Confidence Interval =", "%.5e" % length90SS)
print("CI at 90% =[", "%.5e" % (PFSS-0.5*length90SS) ,
      "; %.5e" % (PFSS+0.5*length90SS) , "]")
print("Threshold = %.5e" % thresholdSS)
print("CI threshold at 90% =[", "%.5e" % (thresholdSS-0.5*thLengthSS) ,
      "; %.5e" % (thresholdSS+0.5*thLengthSS) , "]")
print("Limit state calls =", N_SS)
print("*******************************************************************")
print("")
