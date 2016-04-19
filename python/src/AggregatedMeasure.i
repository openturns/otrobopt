// SWIG file AggregatedMeasure.i

%{
#include "otrobopt/AggregatedMeasure.hxx"
%}

%include AggregatedMeasure_doc.i

TypedCollectionInterfaceObjectMisnamedHelper(OTROBOPT, MeasureEvaluation, MeasureEvaluationCollection)

%include otrobopt/AggregatedMeasure.hxx
namespace OTROBOPT { %extend AggregatedMeasure { AggregatedMeasure(const AggregatedMeasure & other) { return new OTROBOPT::AggregatedMeasure(other); } } }
