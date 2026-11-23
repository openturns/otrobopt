// SWIG file

%{
#include "otrobopt/ExperimentEstimator.hxx"
%}

%include ExperimentEstimator_doc.i

%copyctor OTROBOPT::ExperimentEstimator;
%include otrobopt/ExperimentEstimator.hxx
