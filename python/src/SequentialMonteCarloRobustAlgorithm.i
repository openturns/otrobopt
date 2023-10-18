// SWIG file

%{
#include "otrobopt/SequentialMonteCarloRobustAlgorithm.hxx"
%}

%include SequentialMonteCarloRobustAlgorithm_doc.i

%copyctor OTROBOPT::SequentialMonteCarloRobustAlgorithm;
%include otrobopt/SequentialMonteCarloRobustAlgorithm.hxx
