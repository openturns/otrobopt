%feature("docstring") OTROBOPT::ExperimentEstimator
R"RAW(Sampling estimator of a system-event probability.

The experiment generates a sample :math:`(\theta_i)_{1 \le i \le n}` and
weights :math:`(w_i)_{1 \le i \le n}`, normalized by their sum.  The
probability is the weighted mean of the Boolean predicate evaluated on the
indicators of the limit-state components,

.. math::

    P(x) = \sum_{i=1}^{n} w_i\,
    \varphi\left(\mathbf{1}_{G_1(x, \theta_i) \ge 0}, \dots,
    \mathbf{1}_{G_m(x, \theta_i) \ge 0}\right).

The error estimate is the binomial standard error at the effective sample size
:math:`1 / \sum_i w_i^2`.  It is exact for independent Monte Carlo sampling and
conservative for low-discrepancy or Latin hypercube sampling, whose true
variance is lower.  It is NaN when the effective sample size does not exceed
one.

The estimator has no geometry.  Reproducibility is controlled by the global
random generator seed.

Parameters
----------
experiment : :py:class:`openturns.WeightedExperiment`
    Experiment used to generate the sample.  Its distribution is replaced by
    the distribution passed to the estimator at each call.

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> f = ot.ParametricFunction(
...     ot.SymbolicFunction(["x", "theta"], ["x - theta"]), [1], [0.0]
... )
>>> predicate = ot.SymbolicFunction(["y0"], ["y0"])
>>> estimator = otrobopt.ExperimentEstimator(ot.MonteCarloExperiment(10000))
>>> probability = estimator.getSafeProbability(predicate, f, ot.Normal(), [0.0])
>>> abs(probability - 0.5) < 0.02
True
)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ExperimentEstimator::setExperiment
"Experiment accessor.

Parameters
----------
experiment : :py:class:`openturns.WeightedExperiment`
    Experiment used to generate the sample."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::ExperimentEstimator::getExperiment
"Experiment accessor.

Returns
-------
experiment : :py:class:`openturns.WeightedExperiment`
    Experiment used to generate the sample."
