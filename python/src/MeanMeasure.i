// SWIG file

%{
#include "otrobopt/MeanMeasure.hxx"
%}

%include MeanMeasure_doc.i

%include otrobopt/MeanMeasure.hxx
namespace OTROBOPT { %extend MeanMeasure { MeanMeasure(const MeanMeasure & other) { return new OTROBOPT::MeanMeasure(other); } } }
