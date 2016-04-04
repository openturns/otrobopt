// SWIG file

%{
#include "otrobopt/VarianceMeasure.hxx"
%}

%include VarianceMeasure_doc.i

%include otrobopt/VarianceMeasure.hxx
namespace OTROBOPT { %extend VarianceMeasure { VarianceMeasure(const VarianceMeasure & other) { return new OTROBOPT::VarianceMeasure(other); } } }
