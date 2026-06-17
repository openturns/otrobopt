#!/usr/bin/env python

import openturns as ot
import otrobopt
import os

# Test IndividualChanceMeasure with discrete distribution
# (exercises the branch where weights index was wrong)
ot.RandomGenerator.SetSeed(0)

f_base = ot.SymbolicFunction(['x', 'theta'], ['x*theta'])
f = ot.ParametricFunction(f_base, [1], [1.0])

x = [2.0]

# Discrete distribution with 5 equally-weighted points
discrete_points = [[0.0], [0.5], [1.0], [1.5], [2.0]]
probas = [0.2] * 5
discrete_dist = ot.UserDefined(discrete_points, probas)

icm = otrobopt.IndividualChanceMeasure(f, discrete_dist, ot.Greater(), [0.5])
val = icm(x)
print('IndividualChance (discrete)=', val)

# JointChanceMeasure with discrete distribution
jcm = otrobopt.JointChanceMeasure(f, discrete_dist, ot.Greater(), 0.9)
val2 = jcm(x)
print('JointChance (discrete)=', val2)

# QuantileMeasure with discrete distribution
qm = otrobopt.QuantileMeasure(f, discrete_dist, 0.5)
val3 = qm(x)
print('Quantile (discrete)=', val3)

# MeanStandardDeviationTradeoffMeasure with discrete distribution
mstd = otrobopt.MeanStandardDeviationTradeoffMeasure(f, discrete_dist, [0.8])
val4 = mstd(x)
print('MeanStdTradeoff (discrete)=', val4)

# VarianceMeasure with discrete distribution
vm = otrobopt.VarianceMeasure(f, discrete_dist)
val5 = vm(x)
print('Variance (discrete)=', val5)

# AggregatedMeasure with discrete distributions
measures = otrobopt.AggregatedMeasure(
    [otrobopt.MeanMeasure(f, discrete_dist),
     otrobopt.VarianceMeasure(f, discrete_dist)])
val6 = measures(x)
print('Aggregated (discrete)=', val6)

# Multi-output IndividualChanceMeasure with discrete distribution
f_base2 = ot.SymbolicFunction(['x', 'theta'], ['x*theta', 'x+theta'])
f2 = ot.ParametricFunction(f_base2, [1], [1.0])
icm2 = otrobopt.IndividualChanceMeasure(f2, discrete_dist, ot.Greater(), [0.5, 0.5])
val7 = icm2(x)
print('IndividualChance (multi-output, discrete)=', val7)

# Test save/load of measures with discrete distributions
filename = 'test_discrete.xml'
study = ot.Study()
study.setStorageManager(ot.XMLStorageManager(filename))
study.add('icm', icm)
study.add('jcm', jcm)
study.save()

study2 = ot.Study()
study2.setStorageManager(ot.XMLStorageManager(filename))
study2.load()
icm_loaded = otrobopt.IndividualChanceMeasure()
study2.fillObject('icm', icm_loaded)
print('IndividualChance loaded=', icm_loaded)
os.remove(filename)
