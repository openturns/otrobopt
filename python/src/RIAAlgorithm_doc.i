%feature("docstring") OTROBOPT::RIAAlgorithm
"Reliability Index Approach (RIA) for RBDO.

Solves an RBDO problem by using FORM to compute the reliability index
for each probabilistic constraint. The outer optimization minimizes
f(d, p) subject to beta(d) - beta_target >= 0.

Parameters
----------
problem : :class:`~otrobopt.RBDOProblem`
    Robust optimization problem
solver : :py:class:`openturns.OptimizationAlgorithm`
    Optimization solver

See also
--------
PMAAlgorithm, SORAAlgorithm"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RIAAlgorithm::setRBDOProblem
"RBDO problem accessor.

Parameters
----------
problem : :class:`~otrobopt.RBDOProblem`
    RBDO problem"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RIAAlgorithm::getRBDOProblem
"RBDO problem accessor.

Returns
-------
problem : :class:`~otrobopt.RBDOProblem`"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RIAAlgorithm::run
"Run the RIA algorithm."
