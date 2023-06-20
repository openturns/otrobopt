// SWIG file InverseFORM.i

%{
#include "otrobopt/InverseFORM.hxx"
%}

%include InverseFORM_doc.i

%include otrobopt/InverseFORM.hxx

namespace OTROBOPT { %extend InverseFORM { InverseFORM(const InverseFORM & other) { return new OTROBOPT::InverseFORM(other); } } }

