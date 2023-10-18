// SWIG file

%{
#include "otrobopt/VarianceMeasure.hxx"
%}

%include VarianceMeasure_doc.i

%copyctor OTROBOPT::VarianceMeasure;
%include otrobopt/VarianceMeasure.hxx
