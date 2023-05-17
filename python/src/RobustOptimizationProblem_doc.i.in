%feature("docstring") OTROBOPT::RobustOptimizationProblem
"Robust optimization problem.

Must consist at least of one :class:`~otrobopt.MeasureEvaluation`, either on
the objective or the constraint.

Available constructors:

    RobustOptimizationProblem(*robustnessMeasure, reliabilityMeasure*)

    RobustOptimizationProblem(*objective, reliabilityMeasure*)

    RobustOptimizationProblem(*robustnessMeasure, inequalityConstraint*)

Parameters
----------
robustnessMeasure : :class:`~otrobopt.MeasureEvaluation`
    Measure function on the objective.
reliabilityMeasure : :class:`~otrobopt.MeasureEvaluation`
    Mesure function on the constraints
objective : :py:class:`openturns.Function`
    Objective function
inequalityConstraint : :py:class:`openturns.Function`
    Inequality constraint

Examples
--------
>>> import openturns as ot
>>> import otrobopt


Pure robustness problem:

>>> thetaDist = ot.Normal(2.0, 0.1)
>>> f_base = ot.SymbolicFunction(['x1', 'theta'], ['x1*theta+cos(x1+theta)'])
>>> f = ot.ParametricFunction(f_base, [1], [1.0])
>>> robustnessMeasure = otrobopt.VarianceMeasure(f, thetaDist) 
>>> problem = otrobopt.RobustOptimizationProblem(robustnessMeasure, ot.Function())
>>> algo = ot.Cobyla(problem)
>>> algo.setStartingPoint([0.0]*robustnessMeasure.getInputDimension())
>>> algo.run()
>>> x_et = algo.getResult().getOptimalPoint()
>>> print(x_et)
[0.55...]
>>> J_et = algo.getResult().getOptimalValue()
>>> print(J_et)
[3.42...e-05]

Robustness problem with deterministic constraint:

>>> thetaDist = ot.Normal(2.0, 0.1)
>>> f_base = ot.SymbolicFunction(['x1', 'theta'], ['x1*theta'])
>>> f = ot.ParametricFunction(f_base, [1], [1.0])
>>> robustnessMeasure = otrobopt.MeanMeasure(f, thetaDist)
>>> g = ot.SymbolicFunction(['x1'], ['x1 - 5.0'])
>>> problem = otrobopt.RobustOptimizationProblem(robustnessMeasure, g)
>>> algo = ot.Cobyla(problem)
>>> algo.setStartingPoint([0.0]*robustnessMeasure.getInputDimension())
>>> algo.run()
>>> x_et = algo.getResult().getOptimalPoint()
>>> J_et = algo.getResult().getOptimalValue()

Pure reliability problem:

>>> thetaDist = ot.Normal(2.0, 0.1)
>>> f = ot.SymbolicFunction(['x1'], ['x1+1'])
>>> g_base = ot.SymbolicFunction(['x1', 'theta'], ['x1+theta'])
>>> g = ot.ParametricFunction(g_base, [1], [1.0])
>>> reliabilityMeasure = otrobopt.JointChanceMeasure(g, thetaDist, ot.Greater(), 0.95)
>>> problem = otrobopt.RobustOptimizationProblem(f, reliabilityMeasure)
>>> algo = ot.Cobyla(problem)
>>> algo.setStartingPoint([0.0]*robustnessMeasure.getInputDimension())
>>> algo.run()
>>> x_et = algo.getResult().getOptimalPoint()
>>> print(x_et)
[-1.83...]
>>> J_et = algo.getResult().getOptimalValue()
>>> print(J_et)
[-0.83...]

Robustness & reliability problem:

>>> thetaDist = ot.Normal(2.0, 0.1)
>>> f_base = ot.SymbolicFunction(['x1', 'theta'], ['x1*theta'])
>>> f = ot.ParametricFunction(f_base, [1], [1.0])
>>> robustnessMeasure = otrobopt.MeanMeasure(f, thetaDist)
>>> g_base = ot.SymbolicFunction(['x1', 'theta'], ['x1+theta'])
>>> g = ot.ParametricFunction(g_base, [1], [1.0])
>>> reliabilityMeasure = otrobopt.JointChanceMeasure(g, thetaDist, ot.Greater(), 0.95)
>>> problem = otrobopt.RobustOptimizationProblem(robustnessMeasure, reliabilityMeasure)
>>> algo = ot.Cobyla(problem)
>>> algo.setStartingPoint([0.0]*robustnessMeasure.getInputDimension())
>>> algo.run()
>>> x_et = algo.getResult().getOptimalPoint()
>>> print(x_et)
[-1.83...]
>>> J_et = algo.getResult().getOptimalValue()
>>> print(J_et)
[-3.67...]"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RobustOptimizationProblem::setRobustnessMeasure
"Robustness measure accessor.

The measure on associated to the objective function

Parameters
----------
robustnessMeasure : :class:`~otrobopt.MeasureEvaluation`
    Measure function on the objective."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RobustOptimizationProblem::getRobustnessMeasure
"Robustness measure accessor.

The measure on associated to the objective function

Returns
-------
robustnessMeasure : :class:`~otrobopt.MeasureEvaluation`
    Measure function on the objective."

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RobustOptimizationProblem::hasRobustnessMeasure
"Robustness measure flag accessor.

If no robustness measure was set, the problem should have a
reliability measure.
The objective is deduced from the robustness measure.

Returns
-------
hasReliabilityMeasure : bool
    Whether a robustness measure was set"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RobustOptimizationProblem::setReliabilityMeasure
"Reliability measure accessor.

The measure on associated to the constraint

Parameters
----------
reliabilityMeasure : :class:`~otrobopt.MeasureEvaluation`
    Mesure function on the constraints"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RobustOptimizationProblem::getReliabilityMeasure
"Reliability measure accessor.

The measure on associated to the constraint

Returns
-------
reliabilityMeasure : :class:`~otrobopt.MeasureEvaluation`
    Mesure function on the constraints"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RobustOptimizationProblem::hasReliabilityMeasure
"Reliability measure flag accessor.

If no reliability measure was set, the problem should have a
robustness measure.
The constraints are deduced from the reliability measure.

Returns
-------
hasReliabilityMeasure : bool
    Whether a reliability measure was set"

// ---------------------------------------------------------------------

%feature("docstring") OTROBOPT::RobustOptimizationProblem::getDistribution
"Distribution accessor.

Returns
-------
distribution : :py:class:`openturns.Distribution`
    Parameter distribution"
