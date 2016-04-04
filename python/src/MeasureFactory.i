// SWIG file

%{
#include "otrobopt/MeasureFactory.hxx"
%}

%include MeasureFactory_doc.i

%include otrobopt/MeasureFactory.hxx
namespace OTROBOPT { %extend MeasureFactory { MeasureFactory(const MeasureFactory & other) { return new OTROBOPT::MeasureFactory(other); } } }
