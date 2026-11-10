// SWIG file RobustOptimizationProblemCanonicalizer.i

%{
#include "otrobopt/RobustOptimizationProblemCanonicalizer.hxx"
%}

%include RobustOptimizationProblemCanonicalizer_doc.i

%copyctor OTROBOPT::CanonicalizationOptions;
%include otrobopt/RobustOptimizationProblemCanonicalizer.hxx

%pythoncode %{
def _CanonicalizationOptions_reference_get(self):
    return self.getReference()

def _CanonicalizationOptions_reference_set(self, value):
    self.setReference(value)

def _CanonicalizationOptions_parameterBounds_get(self):
    return self.getParameterBounds()

def _CanonicalizationOptions_parameterBounds_set(self, value):
    self.setParameterBounds(value)

def _CanonicalizationOptions_parameterStart_get(self):
    return self.getParameterStart()

def _CanonicalizationOptions_parameterStart_set(self, value):
    self.setParameterStart(value)

def _CanonicalizationOptions_activeParameter_get(self):
    return self.getActiveParameter()

def _CanonicalizationOptions_activeParameter_set(self, value):
    self.setActiveParameter(value)

def _CanonicalizationOptions_optimizeParameter_get(self):
    return self.getOptimizeParameter()

def _CanonicalizationOptions_optimizeParameter_set(self, value):
    self.setOptimizeParameter(value)

def _CanonicalizationOptions_worstCasePolicy_get(self):
    return self.getWorstCasePolicy()

def _CanonicalizationOptions_worstCasePolicy_set(self, value):
    self.setWorstCasePolicy(value)

def _CanonicalizationOptions_worstCaseDomain_get(self):
    return self.getWorstCaseDomain()

def _CanonicalizationOptions_worstCaseDomain_set(self, value):
    self.setWorstCaseDomain(value)

CanonicalizationOptions.reference = property(_CanonicalizationOptions_reference_get,
                                             _CanonicalizationOptions_reference_set)
CanonicalizationOptions.parameterBounds = property(_CanonicalizationOptions_parameterBounds_get,
                                                   _CanonicalizationOptions_parameterBounds_set)
CanonicalizationOptions.parameterStart = property(_CanonicalizationOptions_parameterStart_get,
                                                  _CanonicalizationOptions_parameterStart_set)
CanonicalizationOptions.activeParameter = property(_CanonicalizationOptions_activeParameter_get,
                                                   _CanonicalizationOptions_activeParameter_set)
CanonicalizationOptions.optimizeParameter = property(_CanonicalizationOptions_optimizeParameter_get,
                                                     _CanonicalizationOptions_optimizeParameter_set)
CanonicalizationOptions.worstCasePolicy = property(_CanonicalizationOptions_worstCasePolicy_get,
                                                   _CanonicalizationOptions_worstCasePolicy_set)
CanonicalizationOptions.worstCaseDomain = property(_CanonicalizationOptions_worstCaseDomain_get,
                                                   _CanonicalizationOptions_worstCaseDomain_set)
%}
