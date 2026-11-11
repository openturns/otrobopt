%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm
R"RAW(Sequential optimization and reliability assessment.

This class is experimental and may change in future releases.

The algorithm solves the reliability-based design optimization problem

.. math::

    \min_{x} f(x)
    \quad \text{subject to} \quad
    \ProbCond{\cD}{g(x;\Theta) \ge 0} \ge \alpha,

where :math:`f` is a scalar classical function and the chance constraint is
given by a :class:`~otrobopt.JointChanceMeasure`.  The distribution parameter
is kept fixed.  The joint chance constraint is handled by sequential
optimization and reliability assessment (SORA): a sequence of deterministic
optimizations with shifted constraints alternates with a performance-measure
analysis (PMA) in the standard normal space.

At iteration :math:`k`, let :math:`x^k` be the design point and
:math:`\theta^{*k}` the physical image of the most probable point found by
PMA.  The physical shift is

.. math::

    d^k = x^k - \theta^{*k}.

The deterministic subproblem is

.. math::

    x^{k+1} = \argmin_{x} f(x)
    \quad \text{subject to} \quad
    g(x; x^k - d^k) \ge 0,

that is, :math:`g(x; \theta^{*k}) \ge 0`.  A new PMA is then performed at
:math:`x^{k+1}` and the shift is updated.  The loop stops when both the design
point and the shift change by less than the outer absolute or relative error
tolerances.

The physical shift is only defined when the design dimension and the random
dimension are equal.  A problem with :math:`\dim(x) \neq \dim(\Theta)` is
rejected with an explicit dimension error; use
:class:`~otrobopt.DoubleLoopRBDOAlgorithm` or a sample-based algorithm for
such problems.

A vector limit state is handled with the
:meth:`setJointChanceTreatment` option:

* ``SORARobustOptimizationAlgorithm.EXACT`` uses one scalarized system limit
  state :math:`\sigma \min_j g_j` for the PMA and maintains one shift.  This
  is a FORM approximation of the joint event, not an exact system
  reliability calculation.
* ``SORARobustOptimizationAlgorithm.BONFERRONI`` replaces the joint event by
  component events with the conservative target
  :math:`1-(1-p_s)/n_g`.  Each component has its own PMA and its own shift.

The treatment is always explicit and is reported by
:meth:`getJointChanceTreatment`.  The target index is
:math:`\beta_t=\Phi^{-1}(p_s)`, where :math:`p_s` is the safe-event
probability.  For a ``Greater`` operator the safe event is
:math:`\{g \ge 0\}` and :math:`p_s=\alpha`.  For a ``Less`` operator the safe
event is the complementary event :math:`\{g < 0\}` and
:math:`p_s=1-\alpha`.  The exact and Bonferroni treatments therefore require
:math:`0.5 < p_s < 1`.

Available constructors:

    SORARobustOptimizationAlgorithm(*problem, solver, innerSolver*)

    SORARobustOptimizationAlgorithm(*problem, solver*)

    SORARobustOptimizationAlgorithm()

Parameters
----------
problem : :class:`~otrobopt.RBDOProblem`
    Reliability-based design optimization problem.
solver : :py:class:`openturns.OptimizationAlgorithm`
    Deterministic optimization algorithm used for each shifted subproblem.
innerSolver : :py:class:`openturns.OptimizationAlgorithm`
    Reliability algorithm used by the PMA.  The default is
    :class:`~openturns.Cobyla`.

Notes
-----
The PMA is performed in the standard normal space.  The standard-space limit
state is built from the inverse isoprobabilistic transformation of the
distribution.  The origin of the standard space is the median point of the
distribution.

The algorithm requires the limit-state function to expose a gradient when a
gradient-based deterministic or inner solver is used.  The default
:class:`~openturns.Cobyla` solvers do not require it.

The outer loop uses the inherited controls
``setMaximumIterationNumber``, ``setMaximumAbsoluteError`` and
``setMaximumRelativeError``.  The result status is ``SUCCESS`` when both the
design and the shift converge, ``INTERRUPTION`` when the stop callback
interrupts the loop, and ``FAILURE`` when the maximum number of iterations is
reached first.  The status message reports the selected joint chance
treatment.

The default inner solver tolerances can be changed with the ResourceMap keys
``SORARobustOptimizationAlgorithm-DefaultInnerMaximumIterationNumber``,
``SORARobustOptimizationAlgorithm-DefaultInnerMaximumConstraintError`` and
``SORARobustOptimizationAlgorithm-DefaultInnerMaximumAbsoluteError``.

The result is available through the inherited ``getResult`` method.  The
reliability analysis at the solution is reported by
:meth:`getReliabilityIndex`, :meth:`getPerformanceMeasure`,
:meth:`getShift`, :meth:`getStandardSpaceDesignPoint`,
:meth:`getPhysicalSpaceDesignPoint` and :meth:`getApproximateProbability`.

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
>>> algo = otrobopt.SORARobustOptimizationAlgorithm(
...     problem, ot.Cobyla()
... )
>>> algo.run()
>>> result = algo.getResult()

See also
--------
otrobopt.RBDOProblem, otrobopt.JointChanceMeasure,
otrobopt.DoubleLoopRBDOAlgorithm)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm::setRBDOProblem
R"RAW(Set the reliability-based design optimization problem.

Parameters
----------
problem : :class:`~otrobopt.RBDOProblem`
    Reliability-based design optimization problem.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm::getRBDOProblem
R"RAW(Accessor to the reliability-based design optimization problem.

Returns
-------
problem : :class:`~otrobopt.RBDOProblem`
    Reliability-based design optimization problem.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm::setOptimizationAlgorithm
R"RAW(Set the deterministic optimization algorithm.

Parameters
----------
solver : :py:class:`openturns.OptimizationAlgorithm`
    Deterministic optimization algorithm used for each shifted subproblem.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm::getOptimizationAlgorithm
R"RAW(Accessor to the deterministic optimization algorithm.

Returns
-------
solver : :py:class:`openturns.OptimizationAlgorithm`
    Deterministic optimization algorithm used for each shifted subproblem.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm::setInnerOptimizationAlgorithm
R"RAW(Set the inner reliability algorithm.

Parameters
----------
solver : :py:class:`openturns.OptimizationAlgorithm`
    Reliability algorithm used by the performance-measure analysis.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm::getInnerOptimizationAlgorithm
R"RAW(Accessor to the inner reliability algorithm.

Returns
-------
solver : :py:class:`openturns.OptimizationAlgorithm`
    Reliability algorithm used by the performance-measure analysis.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm::setJointChanceTreatment
R"RAW(Set the joint chance treatment.

Parameters
----------
treatment : int
    Either ``SORARobustOptimizationAlgorithm.EXACT`` for the scalarized joint
    event or ``SORARobustOptimizationAlgorithm.BONFERRONI`` for the
    conservative componentwise split.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm::getJointChanceTreatment
R"RAW(Accessor to the joint chance treatment.

Returns
-------
treatment : int
    Either ``SORARobustOptimizationAlgorithm.EXACT`` or
    ``SORARobustOptimizationAlgorithm.BONFERRONI``.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm::getTargetReliabilityIndex
R"RAW(Target reliability index defined by the chance constraint.

The index is :math:`\Phi^{-1}(p_s)` for the exact treatment and the
Bonferroni component index for the conservative treatment.

Returns
-------
targetIndex : float
    Target reliability index :math:`\beta_t`.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm::getReliabilityIndex
R"RAW(Diagnostic reliability index at the solution.

The index is computed by a reliability index analysis (RIA) at the SORA
solution.  It is negative when the origin of the standard space belongs to
the failure domain.

Returns
-------
reliabilityIndex : float
    Signed reliability index :math:`\beta`.

Raises
------
RuntimeError
    If the algorithm has not been run.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm::getPerformanceMeasure
R"RAW(Performance measure at the solution.

Returns
-------
performanceMeasure : float
    Minimum of the scalarized limit state on the target sphere at the worst
    component of the solution.

Raises
------
RuntimeError
    If the algorithm has not been run.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm::getShift
R"RAW(Physical shift of the worst component at the solution.

The shift is :math:`d=x^*-\theta^*`, where :math:`x^*` is the design point
and :math:`\theta^*` the physical image of the most probable point.  For a
Bonferroni treatment, the shift of the component with the smallest
performance measure is returned.

Returns
-------
shift : :class:`~openturns.Point`
    Physical shift of dimension :math:`\dim(x)`.

Raises
------
RuntimeError
    If the algorithm has not been run.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm::getStandardSpaceDesignPoint
R"RAW(Standard-space design point of the worst component at the solution.

Returns
-------
designPoint : :class:`~openturns.Point`
    Most probable point in the standard normal space.

Raises
------
RuntimeError
    If the algorithm has not been run.)RAW"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm::getPhysicalSpaceDesignPoint
R"RAW(Physical-space design point of the worst component at the solution.

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

%feature("docstring") OTROBOPT::SORARobustOptimizationAlgorithm::getApproximateProbability
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
