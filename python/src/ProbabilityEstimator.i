// SWIG file

%{
#include "otrobopt/ProbabilityEstimator.hxx"
%}

%include ProbabilityEstimator_doc.i

// The generic OTTypedInterfaceObjectHelper macro hardcodes the OT namespace;
// the estimators live in OTROBOPT, so instantiate the lower-level helper.
TypedInterfaceObjectImplementationHelper(OTROBOPT, ProbabilityEstimator, ProbabilityEstimatorImplementation)

%copyctor OTROBOPT::ProbabilityEstimator;
%include otrobopt/ProbabilityEstimator.hxx
