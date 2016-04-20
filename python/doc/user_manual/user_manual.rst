User manual
===========

.. currentmodule:: otrobopt

The goal is to formulate and solve robust optimization problem.

A robust optimization problem consists of a probabilistic objective :math:`J` and/or
a probabilistic constraint :math:`G`.

The probabilistic objective/constraint is a parametric function of a physical
parameters :math:`x` and a probabilistic variable :math:`\theta`.

.. math::

    \underset{x \in \mathbb{R}^n}{\text{minimize~}} J(x, \theta)

    \text{subject to~} G(x, \theta) \geq 0

    a \leq x \leq b

    \theta \thicksim \mathcal{D}


The definition of the the probabilistic objective/constraint is associated to
the concept of :class:`~otrobopt.MeasureEvaluation`.

Note that this measure evalution can be discretized over :math:`\theta` so as
to define a deterministic optimization problem using :class:`~otrobopt.MeasureFactory`.

A robust optimization problem can be defined with
:class:`~otrobopt.RobustOptimizationProblem`, and then solved using a
:class:`~otrobopt.RobustOptimizationAlgorithm`.


Measure function evaluation
---------------------------

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    MeasureEvaluation
    MeanMeasure
    VarianceMeasure
    MeanStandardDeviationTradeoffMeasure
    QuantileMeasure
    WorstCaseMeasure
    AggregatedMeasure
    JointChanceMeasure
    IndividualChanceMeasure

Discretize a measure function
-----------------------------

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    MeasureFactory

Define a robust optimization problem
------------------------------------

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    RobustOptimizationProblem

Solve a robust optimization problem
------------------------------------

.. autosummary::
    :toctree: _generated/
    :template: class.rst_t

    RobustOptimizationAlgorithm
    SequentialMonteCarloRobustAlgorithm