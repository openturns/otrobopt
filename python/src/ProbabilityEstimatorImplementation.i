// SWIG file

%{
#include "otrobopt/ProbabilityEstimatorImplementation.hxx"
%}

%include ProbabilityEstimatorImplementation_doc.i

%ignore OTROBOPT::ProbabilityEstimatorImplementation::ProbabilityEstimatorImplementation;
%ignore OTROBOPT::ProbabilityEstimatorImplementation::computeSafeProbability;
%ignore OTROBOPT::ProbabilityEstimatorImplementation::computeEstimationError;
%ignore OTROBOPT::ProbabilityEstimatorImplementation::checkArguments;

%include otrobopt/ProbabilityEstimatorImplementation.hxx
