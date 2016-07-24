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

template <>
inline
OT::OptimizationResult
convert< _PyObject_, OT::OptimizationResult >(PyObject * pyObj)
{
  void * ptr = 0;
  if (SWIG_IsOK(SWIG_ConvertPtr(pyObj, &ptr, SWIGTYPE_p_OT__OptimizationResult, 0))) {
    OT::OptimizationResult * p_mf = reinterpret_cast< OT::OptimizationResult * >(ptr);
    return *p_mf;
  }
  else {
    throw OT::InvalidArgumentException(HERE) << "Object passed as argument is not convertible to an OptimizationResult";
  }
  return OT::OptimizationResult();
}

}
%}

%include SequentialMonteCarloRobustAlgorithm_doc.i

%template(OptimizationResultCollection) OT::Collection<OT::OptimizationResult>;

%include otrobopt/SequentialMonteCarloRobustAlgorithm.hxx
namespace OTROBOPT { %extend SequentialMonteCarloRobustAlgorithm { SequentialMonteCarloRobustAlgorithm(const SequentialMonteCarloRobustAlgorithm & other) { return new OTROBOPT::SequentialMonteCarloRobustAlgorithm(other); } } }
