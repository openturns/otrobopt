%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm
R"RAW(Double-loop reliability-based design optimization.

This class is experimental and may change in future releases.

The algorithm solves the reliability-based design optimization problem

.. math::

    \min_{x} f(x)
    \quad \text{subject to} \quad
    \ProbCond{\cD}{g(x;\Theta) \ge 0} \ge \alpha,

where :math:`f` is a scalar classical function and the chance constraint is
given by a :class:`~otrobopt.JointChanceMeasure`.  The distribution parameter
is kept fixed.  An outer deterministic optimization calls an inner reliability
analysis in the standard normal space.

Two inner formulations are available through :meth:`setMethod`:

* ``DoubleLoopRBDOAlgorithm.RIA`` minimizes the distance to the limit state.
  The outer constraint is :math:`\beta(x)-\beta_t \ge 0`, where the signed
  reliability index :math:`\beta` is negative when the origin of the standard
  space belongs to the failure domain.
* ``DoubleLoopRBDOAlgorithm.PMA`` minimizes the limit state on the sphere of
  radius :math:`\beta_t`.  The outer constraint is the performance measure at
  the target point.

The target index is :math:`\beta_t=\Phi^{-1}(p_s)`, where :math:`p_s` is the
safe-event probability.  For a ``Greater`` operator the safe event is
:math:`\{g \ge 0\}` and :math:`p_s=\alpha`.  For a ``Less`` operator the safe
event is the complementary event :math:`\{g < 0\}` and
:math:`p_s=1-\alpha`.  Both formulations therefore require
:math:`0.5 < p_s < 1`.

A vector limit state is handled with the
:meth:`setJointChanceTreatment` option.  ``EXACT`` uses the exact joint event:
the intersection :math:`\{\min_j g_j \ge 0\}` for a ``Greater`` operator and
the complementary union for a ``Less`` operator.  ``BONFERRONI`` replaces the
joint event by component events with the conservative target
:math:`1-(1-p_s)/n_g`; it is less tight but avoids a nonsmooth scalarization.
The treatment is always explicit and is reported by
:meth:`getJointChanceTreatment`.

Available constructors:

    DoubleLoopRBDOAlgorithm(*problem, solver, innerSolver*)

    DoubleLoopRBDOAlgorithm(*problem, solver*)

    DoubleLoopRBDOAlgorithm()

Parameters
----------
problem : :class:`~otrobopt.RBDOProblem`
    Reliability-based design optimization problem.
solver : :py:class:`openturns.OptimizationAlgorithm`
    Outer deterministic optimization algorithm.
innerSolver : :py:class:`openturns.OptimizationAlgorithm`
    Inner reliability algorithm applied to the scalarized limit state.  It
    must support the constraints used by the selected method.  The default is
    :class:`~openturns.Cobyla`.

Notes
-----
The inner optimization is performed in the standard normal space.  The
standard-space limit state is built from the inverse isoprobabilistic
transformation of the distribution.  The origin of the standard space is the
median point of the distribution.

The algorithm requires the limit-state function to expose a parameter
gradient when a gradient-based inner solver is used.  The default
:class:`~openturns.Cobyla` inner solver does not require it.

The result is available through the inherited ``getResult`` method.  The
reliability analysis at the solution is reported by
:meth:`getReliabilityIndex`, :meth:`getPerformanceMeasure`,
:meth:`getStandardSpaceDesignPoint`, :meth:`getPhysicalSpaceDesignPoint` and
:meth:`getApproximateProbability`.

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
>>> algo = otrobopt.DoubleLoopRBDOAlgorithm(
...     problem, ot.Cobyla()
... )
>>> algo.setMethod(otrobopt.DoubleLoopRBDOAlgorithm.PMA)
>>> algo.run()
>>> result = algo.getResult()

See also
--------
otrobopt.RBDOProblem, otrobopt.JointChanceMeasure)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::setRBDOProblem
R"RAW(Set the reliability-based design optimization problem.

Parameters
----------
problem : :class:`~otrobopt.RBDOProblem`
    Reliability-based design optimization problem.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::getRBDOProblem
R"RAW(Accessor to the reliability-based design optimization problem.

Returns
-------
problem : :class:`~otrobopt.RBDOProblem`
    Reliability-based design optimization problem.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::setOptimizationAlgorithm
R"RAW(Set the outer deterministic optimization algorithm.

Parameters
----------
solver : :py:class:`openturns.OptimizationAlgorithm`
    Outer optimization algorithm.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::getOptimizationAlgorithm
R"RAW(Accessor to the outer deterministic optimization algorithm.

Returns
-------
solver : :py:class:`openturns.OptimizationAlgorithm`
    Outer optimization algorithm.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::setInnerOptimizationAlgorithm
R"RAW(Set the inner reliability algorithm.

Parameters
----------
solver : :py:class:`openturns.OptimizationAlgorithm`
    Inner reliability algorithm applied to the scalarized limit state.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::getInnerOptimizationAlgorithm
R"RAW(Accessor to the inner reliability algorithm.

Returns
-------
solver : :py:class:`openturns.OptimizationAlgorithm`
    Inner reliability algorithm applied to the scalarized limit state.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::setMethod
R"RAW(Set the inner reliability formulation.

Parameters
----------
method : int
    Either ``DoubleLoopRBDOAlgorithm.RIA`` or
    ``DoubleLoopRBDOAlgorithm.PMA``.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::getMethod
R"RAW(Accessor to the inner reliability formulation.

Returns
-------
method : int
    Either ``DoubleLoopRBDOAlgorithm.RIA`` or
    ``DoubleLoopRBDOAlgorithm.PMA``.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::setJointChanceTreatment
R"RAW(Set the joint chance treatment.

Parameters
----------
treatment : int
    Either ``DoubleLoopRBDOAlgorithm.EXACT`` for the exact joint event or
    ``DoubleLoopRBDOAlgorithm.BONFERRONI`` for the conservative componentwise
    split.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::getJointChanceTreatment
R"RAW(Accessor to the joint chance treatment.

Returns
-------
treatment : int
    Either ``DoubleLoopRBDOAlgorithm.EXACT`` or
    ``DoubleLoopRBDOAlgorithm.BONFERRONI``.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::getTargetReliabilityIndex
R"RAW(Target reliability index defined by the chance constraint.

The index is :math:`\Phi^{-1}(p_s)` for the exact treatment and the
Bonferroni component index for the conservative treatment.

Returns
-------
targetIndex : float
    Target reliability index :math:`\beta_t`.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::getReliabilityIndex
R"RAW(Signed reliability index at the solution.

Returns
-------
reliabilityIndex : float
    Reliability index :math:`\beta`.  It is negative when the origin of the
    standard space belongs to the failure domain.

Raises
------
RuntimeError
    If the algorithm has not been run.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::getPerformanceMeasure
R"RAW(Performance measure at the solution.

Returns
-------
performanceMeasure : float
    Minimum of the scalarized limit state on the target sphere.

Raises
------
RuntimeError
    If the algorithm has not been run.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::getStandardSpaceDesignPoint
R"RAW(Standard-space design point at the solution.

Returns
-------
designPoint : :class:`~openturns.Point`
    Most probable point in the standard normal space.

Raises
------
RuntimeError
    If the algorithm has not been run.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::getPhysicalSpaceDesignPoint
R"RAW(Physical-space design point at the solution.

Returns
-------
designPoint : :class:`~openturns.Point`
    Image of the standard-space design point by the inverse isoprobabilistic
    transformation.

Raises
------
RuntimeError
    If the algorithm has not been run.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::DoubleLoopRBDOAlgorithm::getApproximateProbability
R"RAW(FORM approximation of the safe probability at the solution.

Returns
-------
probability : float
    The value :math:`\Phi(\beta)`.  It is not available when no finite
    reliability index could be computed.

Raises
------
RuntimeError
    If the algorithm has not been run.)RAW"
