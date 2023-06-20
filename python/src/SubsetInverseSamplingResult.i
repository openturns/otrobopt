// SWIG file SubsetInverseSamplingResult.i

%{
#include "otrobopt/SubsetInverseSamplingResult.hxx"
%}

%include otrobopt/SubsetInverseSamplingResult.hxx
namespace OTROBOPT { %extend SubsetInverseSamplingResult { SubsetInverseSamplingResult(const SubsetInverseSamplingResult & other) { return new OTROBOPT::SubsetInverseSamplingResult(other); } } }
