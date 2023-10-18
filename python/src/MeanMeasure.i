// SWIG file

%{
#include "otrobopt/MeanMeasure.hxx"
%}

%include MeanMeasure_doc.i

%copyctor OTROBOPT::MeanMeasure;
%include otrobopt/MeanMeasure.hxx
