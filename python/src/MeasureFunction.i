// SWIG file MeasureFunction.i

%{
#include "otrobopt/MeasureFunction.hxx"
#include "openturns/PythonWrappingFunctions.hxx"

namespace OT {
template <>
struct traitsPythonType< OTROBOPT::MeasureFunction >
{
  typedef _PyObject_ Type;
};

template <>
inline
OTROBOPT::MeasureFunction
convert< _PyObject_, OTROBOPT::MeasureFunction >(PyObject * pyObj)
{
  void * ptr = 0;
  if (SWIG_IsOK(SWIG_ConvertPtr(pyObj, &ptr, SWIGTYPE_p_OTROBOPT__MeasureFunction, 0))) {
    OTROBOPT::MeasureFunction * p_mf = reinterpret_cast< OTROBOPT::MeasureFunction * >(ptr);
    return *p_mf;
  } else if (SWIG_IsOK(SWIG_ConvertPtr( pyObj, &ptr, SWIGTYPE_p_OTROBOPT__MeasureFunctionImplementation, 0))) {
    OTROBOPT::MeasureFunctionImplementation * p_impl = reinterpret_cast< OTROBOPT::MeasureFunctionImplementation * >(ptr);
    return *p_impl;
  } else if (SWIG_IsOK(SWIG_ConvertPtr( pyObj, &ptr, SWIG_TypeQuery("OT::Pointer<OTROBOPT::MeasureFunctionImplementation> *"), 0))) {
    OT::Pointer<OTROBOPT::MeasureFunctionImplementation> * p_impl = reinterpret_cast< OT::Pointer<OTROBOPT::MeasureFunctionImplementation> * >(ptr);
    return **p_impl;
  }
  else {
    throw OT::InvalidArgumentException(HERE) << "Object passed as argument is not convertible to a MeasureFunction";
  }
  return OTROBOPT::MeasureFunction();
}

template <>
inline
bool
canConvert< _PyObject_, OTROBOPT::MeasureFunction >(PyObject * pyObj)
{
  void * ptr = 0;
  if (SWIG_IsOK(SWIG_ConvertPtr(pyObj, &ptr, SWIGTYPE_p_OTROBOPT__MeasureFunction, 0))) {
    OTROBOPT::MeasureFunction * p_mf = reinterpret_cast< OTROBOPT::MeasureFunction * >(ptr);
    return p_mf != NULL;
  } else if (SWIG_IsOK(SWIG_ConvertPtr( pyObj, &ptr, SWIGTYPE_p_OTROBOPT__MeasureFunctionImplementation, 0))) {
    OTROBOPT::MeasureFunctionImplementation * p_impl = reinterpret_cast< OTROBOPT::MeasureFunctionImplementation * >(ptr);
    return p_impl != NULL;
  } else if (SWIG_IsOK(SWIG_ConvertPtr( pyObj, &ptr, SWIG_TypeQuery("OT::Pointer<OTROBOPT::MeasureFunctionImplementation> *"), 0))) {
    OT::Pointer<OTROBOPT::MeasureFunctionImplementation> * p_impl = reinterpret_cast< OT::Pointer<OTROBOPT::MeasureFunctionImplementation> * >(ptr);
    return p_impl != NULL && !p_impl->isNull();
  }
  return false;
}

} // OT

%}

%include MeasureFunction_doc.i

TypedInterfaceObjectImplementationHelper(OTROBOPT, MeasureFunction, MeasureFunctionImplementation)

%include otrobopt/MeasureFunction.hxx
namespace OTROBOPT { %extend MeasureFunction { MeasureFunction(const MeasureFunction & other) { return new OTROBOPT::MeasureFunction(other); } } }
