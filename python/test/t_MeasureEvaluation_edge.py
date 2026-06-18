#!/usr/bin/env python

import openturns as ot
import openturns.testing as ott
import otrobopt
import os
import tempfile

# Test IndividualChanceMeasure with discrete distribution
# (exercises the branch where weights index was wrong)
ot.RandomGenerator.SetSeed(0)

f_base = ot.SymbolicFunction(["x", "theta"], ["x*theta"])
f = ot.ParametricFunction(f_base, [1], [1.0])

x = [2.0]

# Discrete distribution with 5 equally-weighted points
discrete_points = [[0.0], [0.5], [1.0], [1.5], [2.0]]
probas = [0.2] * 5
discrete_dist = ot.UserDefined(discrete_points, probas)

icm = otrobopt.IndividualChanceMeasure(f, discrete_dist, ot.Greater(), [0.5])
val = icm(x)
ott.assert_almost_equal(val, [8.12589], 1e-4, 1e-4)

# JointChanceMeasure with discrete distribution
jcm = otrobopt.JointChanceMeasure(f, discrete_dist, ot.Greater(), 0.9)
val2 = jcm(x)
ott.assert_almost_equal(val2, [6.84434], 1e-4, 1e-4)

# QuantileMeasure with discrete distribution
qm = otrobopt.QuantileMeasure(f, discrete_dist, 0.5)
val3 = qm(x)
ott.assert_almost_equal(val3, [2.0], 1e-4, 1e-4)

# MeanStandardDeviationTradeoffMeasure with discrete distribution
mstd = otrobopt.MeanStandardDeviationTradeoffMeasure(f, discrete_dist, [0.8])
val4 = mstd(x)
ott.assert_almost_equal(val4, [1.53137], 1e-4, 1e-4)

# VarianceMeasure with discrete distribution
vm = otrobopt.VarianceMeasure(f, discrete_dist)
val5 = vm(x)
ott.assert_almost_equal(val5, [2.0], 1e-4, 1e-4)

# AggregatedMeasure with discrete distributions
measures = otrobopt.AggregatedMeasure(
    [otrobopt.MeanMeasure(f, discrete_dist), otrobopt.VarianceMeasure(f, discrete_dist)]
)
val6 = measures(x)
ott.assert_almost_equal(val6, [2.0, 2.0], 1e-4, 1e-4)

# Multi-output IndividualChanceMeasure with discrete distribution
f_base2 = ot.SymbolicFunction(["x", "theta"], ["x*theta", "x+theta"])
f2 = ot.ParametricFunction(f_base2, [1], [1.0])
icm2 = otrobopt.IndividualChanceMeasure(f2, discrete_dist, ot.Greater(), [0.5, 0.5])
val7 = icm2(x)
ott.assert_almost_equal(val7.getDimension(), 2)

# Test save/load of measures with discrete distributions
filename = tempfile.NamedTemporaryFile(suffix=".xml", delete=False).name
try:
    study = ot.Study()
    study.setStorageManager(ot.XMLStorageManager(filename))
    study.add("icm", icm)
    study.add("jcm", jcm)
    study.save()

    study2 = ot.Study()
    study2.setStorageManager(ot.XMLStorageManager(filename))
    study2.load()
    icm_loaded = otrobopt.IndividualChanceMeasure()
    study2.fillObject("icm", icm_loaded)
    ott.assert_almost_equal(icm_loaded(x), val, 1e-4, 1e-4)
finally:
    if os.path.exists(filename):
        os.remove(filename)
