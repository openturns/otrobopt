// SWIG file RBDOProblem.i

%{
#include "otrobopt/RBDOProblem.hxx"
%}

%include RBDOProblem_doc.i

%copyctor OTROBOPT::RBDOProblem;
%include otrobopt/RBDOProblem.hxx
