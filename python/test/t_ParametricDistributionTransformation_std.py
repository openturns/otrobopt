#!/usr/bin/env python

import os

import openturns as ot
import openturns.testing as ott
import otrobopt

# 1-D normal: automatic standard reference and live parameter
d = ot.Normal(2.0, 3.0)
T = otrobopt.ParametricDistributionTransformation(d)
assert T.usesStandardSpace()
ott.assert_almost_equal(T.getInputDimension(), 1)
ott.assert_almost_equal(T.getOutputDimension(), 1)
ott.assert_almost_equal(T.getParameter(), [2.0, 3.0])
assert T.getParameterDescription() == ["mu_0", "sigma_0"]
ott.assert_almost_equal(T([0.0]), [2.0])
ott.assert_almost_equal(T([1.0]), [5.0])
ott.assert_almost_equal(T([[0.0], [1.0]]), [[2.0], [5.0]])
ott.assert_almost_equal(T.gradient([1.0]), ot.Matrix([[3.0]]))

# The parameter gradient is (1, xi) for T(xi) = mu + sigma * xi
ott.assert_almost_equal(T.parameterGradient([0.0]), ot.Matrix([[1.0], [0.0]]))
ott.assert_almost_equal(T.parameterGradient([1.0]), ot.Matrix([[1.0], [1.0]]))

# setParameter updates the map but not the original distribution
T.setParameter([5.0, 7.0])
ott.assert_almost_equal(T.getParameter(), [5.0, 7.0])
ott.assert_almost_equal(T([0.0]), [5.0])
ott.assert_almost_equal(T([1.0]), [12.0])
ott.assert_almost_equal(d.getParameter(), [2.0, 3.0])

# The returned distribution is a copy
T.getDistribution().setParameter([1.0, 2.0])
ott.assert_almost_equal(T.getParameter(), [5.0, 7.0])
ott.assert_almost_equal(T([0.0]), [5.0])

# The reference is the fixed standard normal, not a copy of the live distribution
assert T.getReferenceDistribution() == ot.Normal(1)
ott.assert_almost_equal(T.getReferenceDistribution().getDimension(), 1)

# Student: the standard distribution depends on nu, so the generic path is used
student = ot.Student(5.0, 2.0, 3.0)
Ts = otrobopt.ParametricDistributionTransformation(student)
assert not Ts.usesStandardSpace()
assert Ts.getReferenceDistribution() == ot.Normal(1)
ott.assert_almost_equal(Ts.getParameter(), [5.0, 2.0, 3.0])
ott.assert_almost_equal(Ts([0.0]), [2.0])

# The scale is live in the generic path
Ts.setParameter([8.0, 1.0, 2.0])
ott.assert_almost_equal(Ts([0.0]), [1.0])

# Explicit uniform reference: exact pushforward in dimension one
Tsu = otrobopt.ParametricDistributionTransformation(
    ot.Student(5.0, 2.0, 3.0), ot.Uniform(0.0, 1.0)
)
assert not Tsu.usesStandardSpace()
xi = Tsu.getReferenceDistribution().getSample(20000)
theta = Tsu(xi)
ott.assert_almost_equal(theta.computeMean(), [2.0], 0.08, 0.08)
expectedStd = 3.0 * (5.0 / 3.0) ** 0.5
ott.assert_almost_equal(theta.computeStandardDeviation()[0], expectedStd, 0.08, 0.08)
Tsu.setParameter([8.0, 1.0, 2.0])
theta = Tsu(xi)
ott.assert_almost_equal(theta.computeMean(), [1.0], 0.08, 0.08)
ott.assert_almost_equal(
    theta.computeStandardDeviation()[0], 2.0 * (8.0 / 6.0) ** 0.5, 0.08, 0.08
)

# 2-D correlated normal: standard fast path with a live correlation
corr = ot.CorrelationMatrix(2)
corr[0, 1] = 0.4
normal2 = ot.Normal([2.0, 3.0], [1.0, 2.0], corr)
T2 = otrobopt.ParametricDistributionTransformation(normal2)
assert T2.usesStandardSpace()
ott.assert_almost_equal(T2.getParameter(), [2.0, 1.0, 3.0, 2.0, 0.4])
ott.assert_almost_equal(T2([0.0, 0.0]), [2.0, 3.0])
T2.setParameter([5.0, 7.0, 6.0, 8.0, 0.7])
ott.assert_almost_equal(T2([0.0, 0.0]), [5.0, 6.0])
theta2 = T2(T2.getReferenceDistribution().getSample(20000))
ott.assert_almost_equal(theta2.computeMean(), [5.0, 6.0], 0.1, 0.1)
covariance = theta2.computeCovariance()
ott.assert_almost_equal(covariance[0, 0], 49.0, 1.5, 1.5)
ott.assert_almost_equal(covariance[1, 1], 64.0, 2.0, 2.0)
ott.assert_almost_equal(covariance[0, 1] / (7.0 * 8.0), 0.7, 0.03, 0.03)

# Uniform: automatic standard reference pushes a normal sample to the uniform
uniform = ot.Uniform(1.0, 3.0)
Tu = otrobopt.ParametricDistributionTransformation(uniform)
assert Tu.usesStandardSpace()
ott.assert_almost_equal(Tu([0.0]), [2.0])
thetaU = Tu(Tu.getReferenceDistribution().getSample(20000))
ott.assert_almost_equal(thetaU.getMin(), [1.0], 0.02, 0.02)
ott.assert_almost_equal(thetaU.getMax(), [3.0], 0.02, 0.02)
ott.assert_almost_equal(thetaU.computeMean(), [2.0], 0.02, 0.02)

# Explicit reference: a different matching reference is accepted
Texplicit = otrobopt.ParametricDistributionTransformation(
    ot.Normal(2.0, 3.0), ot.Normal(0.0, 1.0)
)
assert Texplicit.usesStandardSpace()
ott.assert_almost_equal(Texplicit([0.0]), [2.0])

# Invalid arguments
with ott.assert_raises(Exception):
    otrobopt.ParametricDistributionTransformation(ot.Normal(2.0, 3.0), ot.Uniform(0.0, 1.0))
with ott.assert_raises(Exception):
    T.setParameter([1.0])

# Serialization round-trip
fileName = "myStudyParametricDistributionTransformation.xml"
study = ot.Study()
study.setStorageManager(ot.XMLStorageManager(fileName))
study.add("T", T)
study.save()

study = ot.Study()
study.setStorageManager(ot.XMLStorageManager(fileName))
study.load()
TReloaded = otrobopt.ParametricDistributionTransformation()
study.fillObject("T", TReloaded)
ott.assert_almost_equal(TReloaded.getParameter(), [5.0, 7.0])
ott.assert_almost_equal(TReloaded([0.0]), [5.0])
ott.assert_almost_equal(TReloaded([1.0]), [12.0])
os.remove(fileName)
