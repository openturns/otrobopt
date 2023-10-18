// SWIG file AggregatedMeasure.i

%{
#include "otrobopt/AggregatedMeasure.hxx"
%}

%include AggregatedMeasure_doc.i

TypedCollectionInterfaceObjectMisnamedHelper(OTROBOPT, MeasureEvaluation, MeasureEvaluationCollection)

%copyctor OTROBOPT::AggregatedMeasure;
%include otrobopt/AggregatedMeasure.hxx
