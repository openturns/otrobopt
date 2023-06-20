// SWIG file SubsetInverseSampling.i

%{
#include "otrobopt/SubsetInverseSampling.hxx"
%}

%include SubsetInverseSampling_doc.i

%include otrobopt/SubsetInverseSampling.hxx
namespace OTROBOPT { %extend SubsetInverseSampling { SubsetInverseSampling(const SubsetInverseSampling & other) { return new OTROBOPT::SubsetInverseSampling(other); } } }
