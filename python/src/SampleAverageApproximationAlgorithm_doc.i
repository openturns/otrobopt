%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm
R"RAW(Sample-average approximation for reliability-based design optimization.

This class is experimental and may change in future releases.

The algorithm replaces the chance constraint of an
:class:`~otrobopt.RBDOProblem` by its empirical counterpart on a fixed
calibration sample :math:`\theta_1,\dots,\theta_N`:

.. math::

    \widehat p_N(x)
    = \frac{1}{N}\sum_{i=1}^N
      \mathbf{1}\bigl\{\theta_i \in \mathcal{S}(x)\bigr\}
    \ge p_s,

where :math:`\mathcal{S}(x)` is the safe event and :math:`p_s` is its target
probability.  For a ``Greater`` operator the safe event is
:math:`\{g(x;\theta) \ge 0\}` and :math:`p_s=\alpha`.  For a ``Less``
operator it is the complementary event :math:`\{g(x;\theta) < 0\}` and
:math:`p_s=1-\alpha`.  The empirical probability is used exactly, without a
smooth surrogate; the resulting constraint is a step function and is solved
by a derivative-free solver such as :class:`~openturns.Cobyla`.

After the optimization the safe probability is estimated again on an
independent validation sample, and a Wilson 95% confidence interval is
reported.  The calibration and validation samples are available through
:meth:`getSample` and :meth:`getValidationSample`; a user-supplied sample can
be passed with :meth:`setSample` or :meth:`setValidationSample` and takes
precedence over the corresponding size.

Available constructors:

    SampleAverageApproximationAlgorithm(*problem, solver, sampleSize*)

    SampleAverageApproximationAlgorithm(*problem, solver*)

    SampleAverageApproximationAlgorithm()

Parameters
----------
problem : :class:`~otrobopt.RBDOProblem`
    Reliability-based design optimization problem.
solver : :py:class:`openturns.OptimizationAlgorithm`
    Outer deterministic optimization algorithm.  A derivative-free algorithm
    is recommended because the empirical chance constraint is not
    differentiable.
sampleSize : int
    Size of the calibration sample.  The default is controlled by the
    ResourceMap key ``SampleAverageApproximationAlgorithm-DefaultSampleSize``.
    The validation sample size is controlled by
    ``SampleAverageApproximationAlgorithm-DefaultValidationSampleSize``.

Notes
-----
The distribution parameter is kept fixed.  The algorithm accepts any target
probability in :math:`[0,1]`, including targets for which the reliability
index is not defined.

The calibration sample is generated at the beginning of :meth:`run` and is
kept fixed during the outer optimization.  The validation sample is generated
after the optimization.  Both are drawn from the distribution of the joint
chance measure with the global random generator.  Set the seed before
:meth:`run` for reproducibility, or provide explicit samples.

The Wilson interval is a binomial confidence interval for the validation
estimate.  It quantifies the sampling error of the validation, not the bias
of the calibration sample or the quality of the deterministic solver.

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> limitState = ot.ParametricFunction(
...     ot.SymbolicFunction(["x", "theta"], ["theta - x"]), [1], [0.0]
... )
>>> measure = otrobopt.JointChanceMeasure(
...     limitState, ot.Normal(), ot.GreaterOrEqual(), 0.9
... )
>>> problem = otrobopt.RBDOProblem(
...     ot.SymbolicFunction(["x"], ["x^2"]), measure
... )
>>> algo = otrobopt.SampleAverageApproximationAlgorithm(
...     problem, ot.Cobyla(), 1000
... )
>>> algo.setValidationSampleSize(10000)
>>> algo.run()
>>> result = algo.getResult()
>>> probability = algo.getSafeProbability()

See also
--------
otrobopt.RBDOProblem, otrobopt.JointChanceMeasure,
otrobopt.DoubleLoopRBDOAlgorithm)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm::setRBDOProblem
R"RAW(Set the reliability-based design optimization problem.

Parameters
----------
problem : :class:`~otrobopt.RBDOProblem`
    Reliability-based design optimization problem.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm::getRBDOProblem
R"RAW(Accessor to the reliability-based design optimization problem.

Returns
-------
problem : :class:`~otrobopt.RBDOProblem`
    Reliability-based design optimization problem.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm::setOptimizationAlgorithm
R"RAW(Set the outer deterministic optimization algorithm.

Parameters
----------
solver : :py:class:`openturns.OptimizationAlgorithm`
    Outer optimization algorithm.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm::getOptimizationAlgorithm
R"RAW(Accessor to the outer deterministic optimization algorithm.

Returns
-------
solver : :py:class:`openturns.OptimizationAlgorithm`
    Outer optimization algorithm.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm::setSampleSize
R"RAW(Set the calibration sample size.

Parameters
----------
sampleSize : int
    Size of the calibration sample.  It must be positive.

Notes
-----
An explicit sample set with :meth:`setSample` takes precedence over this
size.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm::getSampleSize
R"RAW(Accessor to the calibration sample size.

Returns
-------
sampleSize : int
    Size of the calibration sample.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm::setSample
R"RAW(Set the calibration sample.

Parameters
----------
sample : :class:`~openturns.Sample`
    Calibration sample of realizations of the uncertain vector.  Its
    dimension must match the distribution dimension.  An empty sample
    restores random generation at the next :meth:`run`.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm::getSample
R"RAW(Accessor to the calibration sample.

Returns
-------
sample : :class:`~openturns.Sample`
    Calibration sample used by the last :meth:`run`, or the explicit sample
    set by the user.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm::setValidationSampleSize
R"RAW(Set the validation sample size.

Parameters
----------
sampleSize : int
    Size of the validation sample.  It must be positive.

Notes
-----
An explicit sample set with :meth:`setValidationSample` takes precedence
over this size.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm::getValidationSampleSize
R"RAW(Accessor to the validation sample size.

Returns
-------
sampleSize : int
    Size of the validation sample.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm::setValidationSample
R"RAW(Set the validation sample.

Parameters
----------
sample : :class:`~openturns.Sample`
    Validation sample of realizations of the uncertain vector.  Its
    dimension must match the distribution dimension.  An empty sample
    restores random generation at the next :meth:`run`.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm::getValidationSample
R"RAW(Accessor to the validation sample.

Returns
-------
sample : :class:`~openturns.Sample`
    Validation sample used by the last :meth:`run`, or the explicit sample
    set by the user.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm::getSafeProbability
R"RAW(Empirical safe probability at the solution on the calibration sample.

Returns
-------
probability : float
    Fraction of the calibration sample that belongs to the safe event at the
    optimal design.

Raises
------
RuntimeError
    If the algorithm has not been run.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm::getValidationSafeProbability
R"RAW(Empirical safe probability at the solution on the validation sample.

Returns
-------
probability : float
    Fraction of the validation sample that belongs to the safe event at the
    optimal design.

Raises
------
RuntimeError
    If the algorithm has not been run.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SampleAverageApproximationAlgorithm::getValidationConfidenceInterval
R"RAW(Wilson confidence interval of the validation safe probability.

Returns
-------
interval : :class:`~openturns.Interval`
    Wilson 95% confidence interval of the validation safe probability.

Raises
------
RuntimeError
    If the algorithm has not been run.)RAW"
