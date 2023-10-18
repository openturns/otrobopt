// SWIG file MeasureEvaluation.i

%{
#include "otrobopt/MeasureEvaluation.hxx"
#include "openturns/PythonWrappingFunctions.hxx"

namespace OT {
template <>
struct traitsPythonType< OTROBOPT::MeasureEvaluation >
{
  typedef _PyObject_ Type;
};

template <>
inline
OTROBOPT::MeasureEvaluation
convert< _PyObject_, OTROBOPT::MeasureEvaluation >(PyObject * pyObj)
{
  void * ptr = 0;
  if (SWIG_IsOK(SWIG_ConvertPtr(pyObj, &ptr, SWIGTYPE_p_OTROBOPT__MeasureEvaluation, 0))) {
    OTROBOPT::MeasureEvaluation * p_mf = reinterpret_cast< OTROBOPT::MeasureEvaluation * >(ptr);
    return *p_mf;
  } else if (SWIG_IsOK(SWIG_ConvertPtr( pyObj, &ptr, SWIGTYPE_p_OTROBOPT__MeasureEvaluationImplementation, 0))) {
    OTROBOPT::MeasureEvaluationImplementation * p_impl = reinterpret_cast< OTROBOPT::MeasureEvaluationImplementation * >(ptr);
    return *p_impl;
  } else if (SWIG_IsOK(SWIG_ConvertPtr( pyObj, &ptr, SWIG_TypeQuery("OT::Pointer<OTROBOPT::MeasureEvaluationImplementation> *"), 0))) {
    OT::Pointer<OTROBOPT::MeasureEvaluationImplementation> * p_impl = reinterpret_cast< OT::Pointer<OTROBOPT::MeasureEvaluationImplementation> * >(ptr);
    return **p_impl;
  }
  else {
    throw OT::InvalidArgumentException(HERE) << "Object passed as argument is not convertible to a MeasureEvaluation";
  }
  return OTROBOPT::MeasureEvaluation();
}

template <>
inline
bool
canConvert< _PyObject_, OTROBOPT::MeasureEvaluation >(PyObject * pyObj)
{
  void * ptr = 0;
  if (SWIG_IsOK(SWIG_ConvertPtr(pyObj, &ptr, SWIGTYPE_p_OTROBOPT__MeasureEvaluation, 0))) {
    OTROBOPT::MeasureEvaluation * p_mf = reinterpret_cast< OTROBOPT::MeasureEvaluation * >(ptr);
    return p_mf != NULL;
  } else if (SWIG_IsOK(SWIG_ConvertPtr( pyObj, &ptr, SWIGTYPE_p_OTROBOPT__MeasureEvaluationImplementation, 0))) {
    OTROBOPT::MeasureEvaluationImplementation * p_impl = reinterpret_cast< OTROBOPT::MeasureEvaluationImplementation * >(ptr);
    return p_impl != NULL;
  } else if (SWIG_IsOK(SWIG_ConvertPtr( pyObj, &ptr, SWIG_TypeQuery("OT::Pointer<OTROBOPT::MeasureEvaluationImplementation> *"), 0))) {
    OT::Pointer<OTROBOPT::MeasureEvaluationImplementation> * p_impl = reinterpret_cast< OT::Pointer<OTROBOPT::MeasureEvaluationImplementation> * >(ptr);
    return p_impl != NULL && !p_impl->isNull();
  }
  return false;
}

} // OT

%}

%include MeasureEvaluation_doc.i

TypedInterfaceObjectImplementationHelper(OTROBOPT, MeasureEvaluation, MeasureEvaluationImplementation)

%copyctor OTROBOPT::MeasureEvaluation;
%include otrobopt/MeasureEvaluation.hxx
