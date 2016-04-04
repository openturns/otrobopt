// SWIG file

%{
#include "otrobopt/WorstCaseMeasure.hxx"
%}

%include WorstCaseMeasure_doc.i

%include otrobopt/WorstCaseMeasure.hxx
namespace OTROBOPT { %extend WorstCaseMeasure { WorstCaseMeasure(const WorstCaseMeasure & other) { return new OTROBOPT::WorstCaseMeasure(other); } } }
