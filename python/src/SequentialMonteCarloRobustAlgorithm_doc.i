%feature("docstring") OTROBOPT::SequentialMonteCarloRobustAlgorithm
"Sequential Monte Carlo robust optimization algorithm.

Solves a robust optimization problem by alternating discretizing
measures and solving deterministic problems.

Measures are discretized using an initial size that can be set with
:py:meth:`setInitialSamplingSize` and a size increment that is set with
:py:meth:`setSamplingSizeIncrement`.

The optimization problems relies on a Multi-Start algorithm from an initial LHS
experiment and an internal solver that can be set by :py:meth:`setOptimizationAlgorithm`.
The ResourceMap key `SequentialMonteCarloRobustAlgorithm-ConvergenceFactor`
can be used to control the convergence criteria of the inner solver.

The algorithm stops when the number of iterations has been reached or the
absolute error is small enough.

Parameters
----------
problem : :class:`~otrobopt.RobustOptimizationProblem`
    Robust optimization problem
solver : :py:class:`openturns.OptimizationAlgorithm`
    Optimization solver"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SequentialMonteCarloRobustAlgorithm::setInitialSamplingSize
R"RAW(Initial sampling size accessor.

Initial size of the discretization of :math:`\theta`.

Parameters
----------
initialSamplingSize : int
    Initial sampling size)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SequentialMonteCarloRobustAlgorithm::getInitialSamplingSize
R"RAW(Initial sampling size accessor.

Initial size of the discretization of :math:`\theta`.

Returns
-------
initialSamplingSize : int
    Initial sampling size)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SequentialMonteCarloRobustAlgorithm::setSamplingSizeIncrement
R"RAW(Sampling size increment accessor.

Sampling size increment of :math:`\theta` as a function of the total size at
the previous iteration.

Parameters
----------
samplingSizeIncrement : :py:class:`openturns.Function`
    Sampling size increment)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SequentialMonteCarloRobustAlgorithm::getSamplingSizeIncrement
R"RAW(Sampling size increment accessor.

Sampling size increment of :math:`\theta` as a function of the total size at
the previous iteration.

Returns
-------
samplingSizeIncrement : :py:class:`openturns.Function`
    Sampling size increment)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SequentialMonteCarloRobustAlgorithm::setInitialSearch
"Multi-start number accessor.

Initial number of start points used.

Problem bounds must be specified when multi-start is used as start points are
drawn uniformly into the bounding box using an LHS experiment.

Parameters
----------
initialSearch : int, 0 by default (no multi-start)
    Multi-start number"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SequentialMonteCarloRobustAlgorithm::getInitialSearch
"Multi-start number accessor.

Initial number of start points used.

Problem bounds must be specified when multi-start is used as start points are
drawn uniformly into the bounding box.

Returns
-------
initialSearch : int, 0 by default (no multi-start)
    Multi-start number"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SequentialMonteCarloRobustAlgorithm::getInitialStartingPoints
"Multi-start optimization starting points accessor.

Optimization starting points during the initial search phase.

Returns
-------
startPoints : :class:`openturns.Sample`
    List of optimization starting points"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SequentialMonteCarloRobustAlgorithm::getResultCollection
"Optimization intermediate results accessor.

Optimization results at each step.

Returns
-------
resultColl : sequence of :class:`openturns.OptimizationResult`
    List of optimization results"
