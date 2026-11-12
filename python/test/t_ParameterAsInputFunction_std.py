#!/usr/bin/env python

import os

import openturns as ot
import openturns.testing as ott
import otrobopt

# Scalar function with a two-dimensional parameter
fBase = ot.SymbolicFunction(
    ["x", "theta0", "theta1"], ["x * theta0 + theta1"]
)
f = ot.ParametricFunction(fBase, [1, 2], [3.0, 4.0])
h = otrobopt.ParameterAsInputFunction(f)

ott.assert_almost_equal(h.getInputDimension(), 3)
ott.assert_almost_equal(h.getOutputDimension(), 1)
ott.assert_almost_equal(h.getParameterDimension(), 0)
assert not h.isParallel()
assert h.getInputDescription() == ["x", "theta0", "theta1"]
ott.assert_almost_equal(h([2.0, 3.0, 4.0]), [10.0])
ott.assert_almost_equal(
    h([[2.0, 3.0, 4.0], [5.0, 6.0, 7.0]]), [[10.0], [37.0]]
)

# Exact gradient in OpenTURNS convention: one row per input, one column per output
# input = (x, theta0, theta1): (theta0, x, 1)
ott.assert_almost_equal(h.gradient([2.0, 3.0, 4.0]), ot.Matrix([[3.0], [2.0], [1.0]]))
hWrap = ot.PythonFunction(h.getInputDimension(), h.getOutputDimension(), h)
fdGradient = ot.CenteredFiniteDifferenceGradient(
    1.0e-5, hWrap.getEvaluation()
).gradient([2.0, 3.0, 4.0])
ott.assert_almost_equal(h.gradient([2.0, 3.0, 4.0]), fdGradient)

# The parameter is now an input: no parameter gradient remains
parameterGradient = h.parameterGradient([2.0, 3.0, 4.0])
ott.assert_almost_equal(parameterGradient.getNbRows(), 0)
ott.assert_almost_equal(parameterGradient.getNbColumns(), 1)
ott.assert_almost_equal(h.getParameter().getDimension(), 0)
with ott.assert_raises(TypeError):
    h.setParameter([1.0])

# Freezing the appended coordinates recovers the original function
p = ot.ParametricFunction(h, [1, 2], [3.0, 4.0])
ott.assert_almost_equal(p([2.0]), f([2.0]))
ott.assert_almost_equal(p.gradient([2.0]), f.gradient([2.0]))
assert p.getParameterDescription() == ["theta0", "theta1"]

# Vector-valued function
fBase2 = ot.SymbolicFunction(
    ["x0", "x1", "theta"], ["x0 + theta", "x1 * theta"]
)
f2 = ot.ParametricFunction(fBase2, [2], [2.0])
h2 = otrobopt.ParameterAsInputFunction(f2)
ott.assert_almost_equal(h2.getInputDimension(), 3)
ott.assert_almost_equal(h2.getOutputDimension(), 2)
ott.assert_almost_equal(h2([1.0, 3.0, 2.0]), [3.0, 6.0])
# input = (x0, x1, theta), output = (x0 + theta, x1 * theta)
ott.assert_almost_equal(
    h2.gradient([1.0, 3.0, 2.0]), ot.Matrix([[1.0, 0.0], [0.0, 2.0], [1.0, 3.0]])
)
# The wrapper installs a dimensioned Hessian delegate, so a marginal of a
# vector-valued wrapper can be built: a NoHessian reports a zero output
# dimension and rejects the marginal.
h2Marginal = ot.Function(h2).getMarginal(0)
ott.assert_almost_equal(h2Marginal([1.0, 3.0, 2.0]), [3.0])
h2Wrap = ot.PythonFunction(h2.getInputDimension(), h2.getOutputDimension(), h2)
fdGradient2 = ot.CenteredFiniteDifferenceGradient(
    1.0e-5, h2Wrap.getEvaluation()
).gradient([1.0, 3.0, 2.0])
ott.assert_almost_equal(h2.gradient([1.0, 3.0, 2.0]), fdGradient2)

# Function without parameter
f3 = ot.SymbolicFunction(["x"], ["x^2"])
h3 = otrobopt.ParameterAsInputFunction(f3)
ott.assert_almost_equal(h3.getInputDimension(), 1)
ott.assert_almost_equal(h3.getOutputDimension(), 1)
ott.assert_almost_equal(h3.getParameterDimension(), 0)
ott.assert_almost_equal(h3([3.0]), [9.0])
ott.assert_almost_equal(h3.gradient([3.0]), ot.Matrix([[6.0]]))
ott.assert_almost_equal(h3.parameterGradient([3.0]).getNbRows(), 0)

# Serialization round-trip
fileName = "myStudyParameterAsInput.xml"
study = ot.Study()
study.setStorageManager(ot.XMLStorageManager(fileName))
study.add("h", h)
study.save()

study = ot.Study()
study.setStorageManager(ot.XMLStorageManager(fileName))
study.load()
hReloaded = otrobopt.ParameterAsInputFunction()
study.fillObject("h", hReloaded)
ott.assert_almost_equal(hReloaded.getInputDimension(), 3)
ott.assert_almost_equal(hReloaded([2.0, 3.0, 4.0]), [10.0])
ott.assert_almost_equal(
    hReloaded.gradient([2.0, 3.0, 4.0]), ot.Matrix([[3.0], [2.0], [1.0]])
)
os.remove(fileName)
