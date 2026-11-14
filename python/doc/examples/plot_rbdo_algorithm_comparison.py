r"""
Comparing SORA with the double-loop algorithm
=============================================

This example solves the same one-dimensional problem with
:class:`~otrobopt.SORARobustOptimizationAlgorithm` and with the two inner
formulations of :class:`~otrobopt.DoubleLoopRBDOAlgorithm`.

The problem is

.. math::

    \min_x x^2
    \quad \text{subject to} \quad
    \mathbb{P}_{\Theta}(\Theta - x \ge 0) \ge 0.9,
    \qquad \Theta \sim \mathcal{N}(0, 1).

The safe event is :math:`\{\Theta \ge x\}`, so its probability is
:math:`1-\Phi(x)`.  The ground truth is available analytically:
:math:`x^*=-\Phi^{-1}(0.9)\approx-1.2816` with objective
:math:`x^{*2}\approx1.6424` and probability :math:`0.9`.

The example reports, for each algorithm, the optimal design, the reliability
index, the approximate probability, and the number of objective and
limit-state evaluations.  All algorithms should reproduce the analytic
solution; the evaluation counts show the cost of the inner reliability loop.
"""

# %%
import openturns as ot
import otrobopt


def build_problem():
    """Return a fresh problem and its memoized objective and limit state."""
    objective = ot.MemoizeFunction(ot.SymbolicFunction(["x"], ["x^2"]))
    objective.disableCache()
    limitState = ot.MemoizeFunction(
        ot.ParametricFunction(
            ot.SymbolicFunction(["x", "theta"], ["theta - x"]), [1], [0.0]
        )
    )
    limitState.disableCache()
    measure = otrobopt.JointChanceMeasure(
        limitState, ot.Normal(), ot.GreaterOrEqual(), 0.9
    )
    problem = otrobopt.RBDOProblem(objective, measure)
    return problem, objective, limitState


def solve_sora():
    problem, objective, limitState = build_problem()
    algo = otrobopt.SORARobustOptimizationAlgorithm(problem, ot.Cobyla())
    algo.setStartingPoint([0.0])
    algo.run()
    return algo, objective, limitState


def solve_double_loop(method):
    problem, objective, limitState = build_problem()
    algo = otrobopt.DoubleLoopRBDOAlgorithm(problem, ot.Cobyla())
    algo.setMethod(method)
    algo.setStartingPoint([0.0])
    algo.run()
    return algo, objective, limitState


# %%
# Solve the problem with SORA and with the double-loop RIA and PMA methods.
runs = [
    ("SORA",) + solve_sora(),
    (
        "Double-loop RIA",
    ) + solve_double_loop(otrobopt.DoubleLoopRBDOAlgorithm.RIA),
    (
        "Double-loop PMA",
    ) + solve_double_loop(otrobopt.DoubleLoopRBDOAlgorithm.PMA),
]

# %%
# Collect the results and the evaluation counts.  The reliability index is the
# diagnostic index at the optimal design, and the approximate probability is
# its FORM probability.
print(
    "Algorithm        | design   | index   | probability"
    " | obj. evals | limit-state evals"
)
print(
    "-----------------+----------+---------+-------------"
    "+------------+------------------"
)
for name, algo, objective, limitState in runs:
    design = algo.getResult().getOptimalPoint()[0]
    index = algo.getReliabilityIndex()
    probability = algo.getApproximateProbability()
    objectiveCalls = objective.getInputHistory().getSize()
    limitStateCalls = limitState.getInputHistory().getSize()
    print(
        f"{name:16s} | {design:8.5f} | {index:7.5f} | {probability:11.5f}"
        f" | {objectiveCalls:10d} | {limitStateCalls:16d}"
    )

# %%
# Analytical ground truth and errors.  The exact design is the 10% quantile of
# the standard normal distribution, and the exact probability at a design x is
# ``1 - Phi(x)``.
normal = ot.Normal()
exactDesign = normal.computeQuantile(0.1)[0]
exactObjective = exactDesign**2
exactProbability = 1.0 - normal.computeCDF(exactDesign)
print()
print("Exact design             :", exactDesign)
print("Exact objective          :", exactObjective)
print("Exact probability        :", exactProbability)
print()

for name, algo, _, _ in runs:
    design = algo.getResult().getOptimalPoint()[0]
    trueProbability = 1.0 - normal.computeCDF(design)
    print("Algorithm                :", name)
    print("Design error             :", abs(design - exactDesign))
    print(
        "Objective error          :",
        abs(algo.getResult().getOptimalValue()[0] - exactObjective),
    )
    print("True probability         :", trueProbability)
    print("Probability error        :", abs(trueProbability - exactProbability))
    print()
