// SWIG file

%{
#include "otrobopt/JointChanceMeasure.hxx"
%}

%include JointChanceMeasure_doc.i

%include otrobopt/JointChanceMeasure.hxx
namespace OTROBOPT { %extend JointChanceMeasure { JointChanceMeasure(const JointChanceMeasure & other) { return new OTROBOPT::JointChanceMeasure(other); } } }
