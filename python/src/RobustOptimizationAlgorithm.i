// SWIG file RobustOptimizationAlgorithm.i

%{
#include "otrobopt/RobustOptimizationAlgorithm.hxx"
%}

%include RobustOptimizationAlgorithm_doc.i

%include otrobopt/RobustOptimizationAlgorithm.hxx
namespace OTROBOPT { %extend RobustOptimizationAlgorithm { RobustOptimizationAlgorithm(const RobustOptimizationAlgorithm & other) { return new OTROBOPT::RobustOptimizationAlgorithm(other); } } }
