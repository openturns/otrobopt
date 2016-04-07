// SWIG file

%{
#include "otrobopt/SequentialMonteCarloRobustAlgorithm.hxx"
%}

%include SequentialMonteCarloRobustAlgorithm_doc.i

%include otrobopt/SequentialMonteCarloRobustAlgorithm.hxx
namespace OTROBOPT { %extend SequentialMonteCarloRobustAlgorithm { SequentialMonteCarloRobustAlgorithm(const SequentialMonteCarloRobustAlgorithm & other) { return new OTROBOPT::SequentialMonteCarloRobustAlgorithm(other); } } }
