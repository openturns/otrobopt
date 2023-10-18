// SWIG file

%{
#include "otrobopt/MeasureFactory.hxx"
%}

%include MeasureFactory_doc.i

%copyctor OTROBOPT::MeasureFactory;
%include otrobopt/MeasureFactory.hxx
