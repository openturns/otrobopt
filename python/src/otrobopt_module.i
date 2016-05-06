// SWIG file

%module(docstring="otrobopt module") otrobopt

%{
#include <openturns/OT.hxx>
#include <openturns/PythonWrappingFunctions.hxx>
%}

// Prerequisites needed
%include typemaps.i
%include exception.i
%ignore *::load(OT::Advocate & adv);
%ignore *::save(OT::Advocate & adv) const;

%import base_module.i
%import uncertainty_module.i

// The new classes
%include otrobopt/OTRobOptprivate.hxx
%include MeasureEvaluationImplementation.i
%include MeasureEvaluation.i
%include MeanMeasure.i
%include VarianceMeasure.i
%include MeanStandardDeviationTradeoffMeasure.i
%include QuantileMeasure.i
%include WorstCaseMeasure.i
%include AggregatedMeasure.i
%include JointChanceMeasure.i
%include IndividualChanceMeasure.i
%include MeasureFunction.i
%include MeasureFactory.i
%include RobustOptimizationProblem.i
%include RobustOptimizationAlgorithm.i
%include SequentialMonteCarloRobustAlgorithm.i

