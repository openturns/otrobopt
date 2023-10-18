// SWIG file

%{
#include "otrobopt/MeasureEvaluationImplementation.hxx"
%}

%include MeasureEvaluationImplementation_doc.i

%copyctor OTROBOPT::MeasureEvaluationImplementation;
%include otrobopt/MeasureEvaluationImplementation.hxx
