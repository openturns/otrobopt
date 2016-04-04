// SWIG file

%{
#include "otrobopt/QuantileMeasure.hxx"
%}

%include QuantileMeasure_doc.i

%include otrobopt/QuantileMeasure.hxx
namespace OTROBOPT { %extend QuantileMeasure { QuantileMeasure(const QuantileMeasure & other) { return new OTROBOPT::QuantileMeasure(other); } } }
