User manual
===========

.. currentmodule:: otrobopt

The goal is to formulate and solve robust optimization problem.

A robust optimization problem consists of a parametric objective objective
:math:`J(x, \theta)` and/or a parametric inequality constraint
:math:`G(x, \theta)` where :math:`x` is a design variable
and :math:`\theta` a parameter.

.. math::

    \begin{aligned}
    & \underset{x \in \Rset^n}{\text{minimize~}}
    & & J(x, \theta) \\
    & \text{subject to}
    & & G(x, \theta) \geq 0\\
    \end{aligned}


The problem is made robust by:

- modelling the parameter :math:`\theta` by the
  the random vector :math:`\Theta` with given distribution :math:`\cD`.

- choosing measure functions :math:`\rho_{J, \cD}` and :math:`\lambda_{G, \cD}`
  for the objective and constraint functions.

The the robust optimization problem reads:

.. math::

    \begin{aligned}
    & \underset{x \in \Rset^n}{\text{minimize~}}
    & & \rho_{J, \cD}(x) \ \\
    & \text{subject to}
    & & \lambda_{G, \cD}(x) \geq 0\\
    \end{aligned}

The definition of the measure functions is associated to
the concept of :class:`~otrobopt.MeasureEvaluation`.

A measure evaluation can be used through :class:`~otrobopt.MeasureFunction`
to expose generic function services.

A robust optimization problem can be defined with
:class:`~otrobopt.RobustOptimizationProblem`, and then solved using a
:class:`~otrobopt.RobustOptimizationAlgorithm`.

Note that this measure evaluation can be discretized over :math:`\theta` so as
to define a deterministic optimization problem using :class:`~otrobopt.MeasureFactory`.


Measure function
----------------

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    MeasureFunction

Parameter as input
------------------

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    ParameterAsInputFunction

Parameter-dependent transformation
----------------------------------

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    ParametricDistributionTransformation

Measure evaluation
------------------

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    MeasureEvaluation

.. autosummary::
    :toctree: _generated/
    :template: MeasureEvaluation.rst_t

    MeanMeasure
    MeanStandardDeviationTradeoffMeasure
    QuantileMeasure
    WorstCaseMeasure
    VarianceMeasure
    JointChanceMeasure
    IndividualChanceMeasure

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    AggregatedMeasure


Define a robust optimization problem
------------------------------------

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    RobustOptimizationProblem

Define a reliability-based design optimization problem
------------------------------------------------------

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    RBDOProblem

Canonicalize a robust optimization problem
------------------------------------------

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    CanonicalizationOptions
    RobustOptimizationProblemCanonicalizer

Discretize a measure function
-----------------------------

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    MeasureFactory

Solve a robust optimization problem
------------------------------------

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    RobustOptimizationAlgorithm
    SequentialMonteCarloRobustAlgorithm

Solve a reliability-based design optimization problem
-----------------------------------------------------

The specialized algorithms accept an :class:`~otrobopt.RBDOProblem` and read the
target probability and the comparison operator from its joint chance measure.

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    DoubleLoopRBDOAlgorithm
    SORARobustOptimizationAlgorithm
    SampleAverageApproximationAlgorithm

:class:`~otrobopt.DoubleLoopRBDOAlgorithm` supports RIA and PMA and accepts
problems where the design and random dimensions differ.
:class:`~otrobopt.SORARobustOptimizationAlgorithm` uses the physical shift
:math:`d=x-\theta^*` and therefore requires equal design and random
dimensions; it rejects a mismatched problem with an explicit error.
:class:`~otrobopt.SampleAverageApproximationAlgorithm` accepts any target
probability and is not restricted to a positive reliability index.

.. FIXME: sphinx.errors.SphinxWarning: .../otrobopt.py:docstring of openturns.analytical.AnalyticalResult.getHasoferReliabilityIndexSensitivity:4:undefined label: sensitivity_form
.. _sensitivity_form:
.. _importance_form:
    
Solve an inverse reliability problem
------------------------------------

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    SubsetInverseSampling
    SubsetInverseSamplingResult
    InverseFORM
    InverseFORMResult
