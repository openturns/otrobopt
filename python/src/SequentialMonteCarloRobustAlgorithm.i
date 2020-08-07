// SWIG file

%{
#include "otrobopt/SequentialMonteCarloRobustAlgorithm.hxx"
#include "openturns/PythonWrappingFunctions.hxx"

namespace OT {
template <>
struct traitsPythonType< OT::OptimizationResult >
{
  typedef _PyObject_ Type;
};

}
%}

%include SequentialMonteCarloRobustAlgorithm_doc.i

%template(OptimizationResultCollection) OT::Collection<OT::OptimizationResult>;

%include otrobopt/SequentialMonteCarloRobustAlgorithm.hxx
namespace OTROBOPT { %extend SequentialMonteCarloRobustAlgorithm { SequentialMonteCarloRobustAlgorithm(const SequentialMonteCarloRobustAlgorithm & other) { return new OTROBOPT::SequentialMonteCarloRobustAlgorithm(other); } } }
