// SWIG file InverseFORMResult.i

%{
#include "otrobopt/InverseFORMResult.hxx"
%}

%include InverseFORMResult_doc.i

%include otrobopt/InverseFORMResult.hxx

namespace OTROBOPT { %extend InverseFORMResult { InverseFORMResult(const InverseFORMResult & other) { return new OTROBOPT::InverseFORMResult(other); } } }
