// SWIG file

%{
#include "otrobopt/MeanStandardDeviationTradeoffMeasure.hxx"
%}

%include MeanStandardDeviationTradeoffMeasure_doc.i

%include otrobopt/MeanStandardDeviationTradeoffMeasure.hxx
namespace OTROBOPT { %extend MeanStandardDeviationTradeoffMeasure { MeanStandardDeviationTradeoffMeasure(const MeanStandardDeviationTradeoffMeasure & other) { return new OTROBOPT::MeanStandardDeviationTradeoffMeasure(other); } } }
