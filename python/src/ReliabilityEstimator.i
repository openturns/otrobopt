// SWIG file

%{
#include "otrobopt/ReliabilityEstimator.hxx"
%}

%include ReliabilityEstimator_doc.i

%copyctor OTROBOPT::ReliabilityEstimator;
%include otrobopt/ReliabilityEstimator.hxx
