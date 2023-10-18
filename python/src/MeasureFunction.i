// SWIG file MeasureFunction.i

%{
#include "otrobopt/MeasureFunction.hxx"
%}

%include MeasureFunction_doc.i

%copyctor OTROBOPT::MeasureFunction;
%include otrobopt/MeasureFunction.hxx
