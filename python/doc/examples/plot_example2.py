"""
Example 2
=========
"""

# %%
# Problem statement:
#
# .. math::
#
#     \begin{aligned}
#     & \underset{x}{\text{minimize}}
#     & & \mathbb{E}_{\cD}(\sqrt{x_0} \sqrt{x_1} \Theta) \\
#     & \text{subject to}
#     & & 2x_1 + 4x_0 - 120 \leq 0 \\
#     & & & \Theta \thicksim \cN(1, 3)
#     \end{aligned}
#
# Solution: :math:`\hat{x} = [15, 30]`

# %%
import openturns as ot
import openturns.testing
import otrobopt

# ot.Log.Show(ot.Log.Info)

calJ = ot.SymbolicFunction(
    ['x0', 'x1', 'theta'], ['sqrt(x0) * sqrt(x1) * theta'])
g = ot.SymbolicFunction(['x0', 'x1'], ['-(2*x1 + 4*x0 -120)'])
J = ot.ParametricFunction(calJ, [2], [1.0])

dim = J.getInputDimension()

solver = ot.Cobyla()
solver.setMaximumIterationNumber(1000)

thetaDist = ot.Normal(1.0, 3.0)
robustnessMeasure = otrobopt.MeanMeasure(J, thetaDist)
problem = otrobopt.RobustOptimizationProblem(robustnessMeasure, g)
problem.setMinimization(False)
bounds = ot.Interval([5.0] * dim, [50.0] * dim)
problem.setBounds(bounds)

algo = otrobopt.SequentialMonteCarloRobustAlgorithm(problem, solver)
algo.setMaximumIterationNumber(10)
algo.setMaximumAbsoluteError(1e-3)
algo.setInitialSamplingSize(10)
algo.setInitialSearch(100)
algo.run()
result = algo.getResult()
# openturns.testing.assert_almost_equal(
# result.getOptimalPoint(), [46.2957, 46.634], 1e-3)
print('x*=', result.getOptimalPoint(),
      'J(x*)=', result.getOptimalValue(),
      'iteration=', result.getIterationNumber())
