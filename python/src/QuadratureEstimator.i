// SWIG file

%{
#include "otrobopt/QuadratureEstimator.hxx"
%}

%include QuadratureEstimator_doc.i

%copyctor OTROBOPT::QuadratureEstimator;
%include otrobopt/QuadratureEstimator.hxx
