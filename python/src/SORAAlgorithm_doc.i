%feature("docstring") OTROBOPT::SORAAlgorithm
"Sequential Optimization and Reliability Analysis (SORA) for RBDO.

Iterates between a deterministic optimization with the probabilistic
constraint evaluated at the MPTP and inverse FORM at the optimal point
to update the MPTP. Converges when the design point stabilizes.

Parameters
----------
problem : :class:`~otrobopt.RobustOptimizationProblem`
    Robust optimization problem
solver : :py:class:`openturns.OptimizationAlgorithm`
    Optimization solver

See also
--------
RIAAlgorithm, PMAAlgorithm"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORAAlgorithm::setMaximumSORAIteration
"Maximum number of SORA iterations.

Parameters
----------
maxIter : int
    Maximum number of iterations"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORAAlgorithm::getMaximumSORAIteration
"Maximum number of SORA iterations.

Returns
-------
maxIter : int"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORAAlgorithm::setSORATolerance
"Convergence tolerance for SORA.

Parameters
----------
tolerance : float
    Convergence tolerance on the design point change"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORAAlgorithm::getSORATolerance
"Convergence tolerance for SORA.

Returns
-------
tolerance : float"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORAAlgorithm::getMostProbableTargetPoints
"Most Probable Target Points accessor.

Returns
-------
mptp : sequence of :py:class:`openturns.Point`
    MPTP for each probabilistic constraint"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORAAlgorithm::setRBDOProblem
"RBDO problem accessor.

Parameters
----------
problem : :class:`~otrobopt.RBDOProblem`
    RBDO problem"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORAAlgorithm::getRBDOProblem
"RBDO problem accessor.

Returns
-------
problem : :class:`~otrobopt.RBDOProblem`"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORAAlgorithm::run
"Run the SORA algorithm."
