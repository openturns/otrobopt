// SWIG file

%{
#include "otrobopt/IndividualChanceMeasure.hxx"
%}

%include IndividualChanceMeasure_doc.i

%include otrobopt/IndividualChanceMeasure.hxx
namespace OTROBOPT { %extend IndividualChanceMeasure { IndividualChanceMeasure(const IndividualChanceMeasure & other) { return new OTROBOPT::IndividualChanceMeasure(other); } } }
