// SWIG file

%{
#include "otrobopt/MeasureEvaluationImplementation.hxx"
%}

%include MeasureEvaluationImplementation_doc.i

%include otrobopt/MeasureEvaluationImplementation.hxx
namespace OTROBOPT { %extend MeasureEvaluationImplementation { MeasureEvaluationImplementation(const MeasureEvaluationImplementation & other) { return new OTROBOPT::MeasureEvaluationImplementation(other); } } }
