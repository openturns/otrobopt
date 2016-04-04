// SWIG file RobustOptimizationProblem.i

%{
#include "otrobopt/RobustOptimizationProblem.hxx"
%}

%include RobustOptimizationProblem_doc.i

%include otrobopt/RobustOptimizationProblem.hxx
namespace OTROBOPT { %extend RobustOptimizationProblem { RobustOptimizationProblem(const RobustOptimizationProblem & other) { return new OTROBOPT::RobustOptimizationProblem(other); } } }
