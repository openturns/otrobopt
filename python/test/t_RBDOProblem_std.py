#!/usr/bin/env python

import openturns as ot
import openturns.testing as ott
import otrobopt


def make_measure(alpha=0.9):
    limit_state = ot.ParametricFunction(
        ot.SymbolicFunction(["x", "theta"], ["theta - x"]), [1], [0.0]
    )
    return otrobopt.JointChanceMeasure(
        limit_state, ot.Normal(), ot.GreaterOrEqual(), alpha
    )


# ---------------------------------------------------------- empty problem
empty = otrobopt.RBDOProblem()
assert not empty.hasJointChanceMeasure()
with ott.assert_raises(Exception):
    empty.check()
with ott.assert_raises(Exception):
    empty.getCanonicalizer()
with ott.assert_raises(Exception):
    empty.getJointChanceMeasure()

# ------------------------------------------------------- problem contract
objective = ot.SymbolicFunction(["x"], ["x^2"])
measure = make_measure()
problem = otrobopt.RBDOProblem(objective, measure)
assert problem.hasJointChanceMeasure()
assert problem.getTargetProbability() == 0.9
assert str(problem.getComparisonOperator()) == ">="
assert problem.getLimitStateFunction().getInputDimension() == 1
assert problem.getDistribution() == ot.Normal()
assert "RBDOProblem" in repr(problem)

# The inherited OptimizationProblem protocol is available
problem.setBounds(ot.Interval([-10.0], [10.0]))
problem.setInequalityConstraint(ot.SymbolicFunction(["x"], ["x + 5.0"]))
problem.setEqualityConstraint(ot.SymbolicFunction(["x"], ["x - 0.5"]))
problem.setVariablesType([ot.OptimizationProblemImplementation.INTEGER])
problem.setMinimization(False)
problem.check()

# ------------------------------------------------- dimension and contract checks
with ott.assert_raises(Exception):
    otrobopt.RBDOProblem(
        ot.SymbolicFunction(["x"], ["x", "x^2"]), make_measure()
    )

limit_state_2d = ot.ParametricFunction(
    ot.SymbolicFunction(["x1", "x2", "theta"], ["theta - x1 - x2"]),
    [2],
    [0.0],
)
measure_2d = otrobopt.JointChanceMeasure(
    limit_state_2d, ot.Normal(), ot.GreaterOrEqual(), 0.9
)
with ott.assert_raises(Exception):
    otrobopt.RBDOProblem(objective, measure_2d)

limit_state_2theta = ot.ParametricFunction(
    ot.SymbolicFunction(["x", "theta1", "theta2"], ["theta1 + theta2 - x"]),
    [1, 2],
    [0.0, 0.0],
)
measure_2theta = make_measure()
measure_2theta.setFunction(limit_state_2theta)
with ott.assert_raises(Exception):
    otrobopt.RBDOProblem(objective, measure_2theta)

measure_discrete = otrobopt.JointChanceMeasure(
    ot.ParametricFunction(
        ot.SymbolicFunction(["x", "theta"], ["theta - x"]), [1], [0.0]
    ),
    ot.Bernoulli(),
    ot.GreaterOrEqual(),
    0.9,
)
with ott.assert_raises(Exception):
    otrobopt.RBDOProblem(objective, measure_discrete)

# A constraint with the wrong input dimension is rejected by the base class
bad = otrobopt.RBDOProblem(objective, make_measure())
with ott.assert_raises(Exception):
    bad.setInequalityConstraint(
        ot.SymbolicFunction(["x1", "x2"], ["x1 + x2"])
    )

# ------------------------------------------------- default canonicalization
options = otrobopt.RBDOProblem.GetDefaultCanonicalizationOptions()
assert not options.getOptimizeParameter()

canonicalizer = problem.getCanonicalizer()
assert not canonicalizer.getOptimizeParameter()
assert canonicalizer.getActiveParameter() == [0, 1]

canonical = problem.getCanonicalProblem()
assert canonical.getDimension() == 1
assert not canonical.isMinimization(0)
assert canonical.getVariablesType() == [ot.OptimizationProblemImplementation.INTEGER]
ott.assert_almost_equal(canonical.getObjective()([-1.5]), [2.25])
ott.assert_almost_equal(canonical.getBounds().getLowerBound(), [-10.0])
ott.assert_almost_equal(canonical.getBounds().getUpperBound(), [10.0])

# The inequality gathers the joint chance index first, then the deterministic
# constraints.  For x=-1.5, P(theta >= -1.5) = Phi(1.5), so the index is
# 1.5 - Phi^-1(0.9).
chance_index = 1.5 - ot.DistFunc.qNormal(0.9)
constraint = canonical.getInequalityConstraint()
assert constraint.getOutputDimension() == 2
ott.assert_almost_equal(constraint([-1.5]), [chance_index, 3.5], 1e-10, 1e-10)
ott.assert_almost_equal(canonical.getEqualityConstraint()([-1.5]), [-2.0])
ott.assert_almost_equal(problem.getCanonicalStartingPoint(), [0.0])
ott.assert_almost_equal(
    problem.getCanonicalDeterministicConstraint()([-1.5]), [3.5]
)

frozen_measure = problem.getCanonicalJointChanceMeasure()
assert frozen_measure.getAlpha() == 0.9
assert str(frozen_measure.getOperator()) == ">="
ott.assert_almost_equal(
    frozen_measure([-1.5]), [chance_index], 1e-10, 1e-10
)

# ------------------------------------------------- parameter optimization
parameter_options = otrobopt.CanonicalizationOptions()
parameter_options.setOptimizeParameter(True)
parameter_options.setParameterBounds(ot.Interval([-1.0, 0.1], [1.0, 10.0]))
parameter_options.setParameterStart([0.5, 2.0])

parameter_canonicalizer = problem.getCanonicalizer(parameter_options)
assert parameter_canonicalizer.getOptimizeParameter()
assert parameter_canonicalizer.getActiveParameter() == [0, 1]

parameter_canonical = problem.getCanonicalProblem(parameter_options)
assert parameter_canonical.getDimension() == 3
ott.assert_almost_equal(
    parameter_canonical.getBounds().getLowerBound(), [-10.0, -1.0, 0.1]
)
ott.assert_almost_equal(
    parameter_canonical.getBounds().getUpperBound(), [10.0, 1.0, 10.0]
)
ott.assert_almost_equal(problem.getCanonicalStartingPoint(parameter_options), [0.0, 0.5, 2.0])

# At (x, mu, sigma) = (-2, 0, 1), the chance index is Phi(2) - Phi^-1(0.9).
chance_index = 2.0 - ot.DistFunc.qNormal(0.9)
parameter_constraint = parameter_canonical.getInequalityConstraint()
assert parameter_constraint.getOutputDimension() == 2
ott.assert_almost_equal(
    parameter_constraint([-2.0, 0.0, 1.0]), [chance_index, 3.0], 1e-10, 1e-10
)
ott.assert_almost_equal(
    parameter_canonical.getEqualityConstraint()([-2.0, 0.0, 1.0]), [-2.5]
)
ott.assert_almost_equal(
    problem.getCanonicalDeterministicConstraint(parameter_options)(
        [-2.0, 0.0, 1.0]
    ),
    [3.0],
)
parameter_measure = problem.getCanonicalJointChanceMeasure(parameter_options)
assert parameter_measure.getAlpha() == 0.9
assert str(parameter_measure.getOperator()) == ">="
ott.assert_almost_equal(
    parameter_measure([-2.0, 0.0, 1.0]), [chance_index], 1e-10, 1e-10
)

# Only the mean is optimized: the dimension is two and sigma stays frozen.
mean_only_options = otrobopt.CanonicalizationOptions()
mean_only_options.setOptimizeParameter(True)
mean_only_options.setActiveParameter([0])
mean_only_canonical = problem.getCanonicalProblem(mean_only_options)
assert mean_only_canonical.getDimension() == 2
ott.assert_almost_equal(
    problem.getCanonicalStartingPoint(mean_only_options), [0.0, 0.0]
)
# phi_0 = (x, mu), theta = mu + sigma * xi with sigma = 1
mean_only_constraint = mean_only_canonical.getInequalityConstraint()
ott.assert_almost_equal(
    mean_only_constraint([-2.0, 0.0]),
    [2.0 - ot.DistFunc.qNormal(0.9), 3.0],
    1e-10,
    1e-10,
)

# ------------------------------------------------------------- copy, clone
clone = otrobopt.RBDOProblem(problem)
assert clone.getTargetProbability() == 0.9
assert str(clone.getJointChanceMeasure().getOperator()) == ">="
assert clone.getBounds() == problem.getBounds()

print("OK")
