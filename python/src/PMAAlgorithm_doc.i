%feature("docstring") OTROBOPT::PMAAlgorithm
"Performance Measure Approach (PMA) for RBDO.

Solves an RBDO problem by using inverse FORM to compute the performance
measure for each probabilistic constraint. The outer optimization minimizes
f(d, p) subject to G(d) >= 0 where G is the limit state value at the MPTP.

Parameters
----------
problem : :class:`~otrobopt.RBDOProblem`
    Robust optimization problem
solver : :py:class:`openturns.OptimizationAlgorithm`
    Optimization solver

See also
--------
RIAAlgorithm, SORAAlgorithm"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::PMAAlgorithm::setRBDOProblem
"RBDO problem accessor.

Parameters
----------
problem : :class:`~otrobopt.RBDOProblem`
    RBDO problem"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::PMAAlgorithm::getRBDOProblem
"RBDO problem accessor.

Returns
-------
problem : :class:`~otrobopt.RBDOProblem`"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::PMAAlgorithm::run
"Run the PMA algorithm."
