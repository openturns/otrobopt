%define OTROBOPT_MeasureEvaluation_doc
"Measure evaluation base class.

Base class for measure function evaluations, allows to define robust
counterparts of parametric objective or constraint functions.

See also
--------
MeanMeasure, VarianceMeasure, MeanStandardDeviationTradeoffMeasure,
WorstCaseMeasure, JointChanceMeasure, IndividualChanceMeasure,
QuantileMeasure, AggregatedMeasure"
%enddef
%feature("docstring") OTROBOPT::MeasureEvaluationImplementation
OTROBOPT_MeasureEvaluation_doc

// ---------------------------------------------------------------------

%define OTROBOPT_MeasureEvaluation_setDistribution_doc
"Distribution accessor.

Parameters
----------
distribution : :py:class:`openturns.Distribution`
    Parameters distribution."
%enddef
%feature("docstring") OTROBOPT::MeasureEvaluationImplementation::setDistribution
OTROBOPT_MeasureEvaluation_setDistribution_doc

// ---------------------------------------------------------------------

%define OTROBOPT_MeasureEvaluation_getDistribution_doc
"Distribution accessor.

Returns
-------
distribution : :py:class:`openturns.Distribution`
    Parameters distribution."
%enddef
%feature("docstring") OTROBOPT::MeasureEvaluationImplementation::getDistribution
OTROBOPT_MeasureEvaluation_getDistribution_doc

// ---------------------------------------------------------------------

%define OTROBOPT_MeasureEvaluation_setFunction_doc
"Function accessor.

Parameters
----------
function : :py:class:`openturns.Function`
    Parametric function."
%enddef
%feature("docstring") OTROBOPT::MeasureEvaluationImplementation::setFunction
OTROBOPT_MeasureEvaluation_setFunction_doc

// ---------------------------------------------------------------------

%define OTROBOPT_MeasureEvaluation_getFunction_doc
"Function accessor.

Returns
-------
function : :py:class:`openturns.Function`
    Parametric function."
%enddef
%feature("docstring") OTROBOPT::MeasureEvaluationImplementation::getFunction
OTROBOPT_MeasureEvaluation_getFunction_doc

// ---------------------------------------------------------------------

%define OTROBOPT_MeasureEvaluation_getOutputDimension_doc
"Accessor to the dimension of the measure values.

Returns
-------
outputDimension : int
    Dimension of the values returned by the measure.  It may differ from
    the output dimension of the underlying function, for example for a
    joint chance measure over a vector-valued function."
%enddef
%feature("docstring") OTROBOPT::MeasureEvaluationImplementation::getOutputDimension
OTROBOPT_MeasureEvaluation_getOutputDimension_doc

// ---------------------------------------------------------------------

%define OTROBOPT_MeasureEvaluation_rebuild_doc
"Rebuild the measure over a new function and distribution.

Parameters
----------
function : :py:class:`openturns.Function`
    Parametric function of the rebuilt measure.
distribution : :py:class:`openturns.Distribution`
    Distribution of the rebuilt measure.
optimizationDomain : :py:class:`openturns.Interval`
    Optional optimization domain, used by the worst-case measure.

Returns
-------
measure : :class:`~otrobopt.MeasureEvaluation`
    The rebuilt measure."
%enddef
%feature("docstring") OTROBOPT::MeasureEvaluationImplementation::rebuild
OTROBOPT_MeasureEvaluation_rebuild_doc

// ---------------------------------------------------------------------

%define OTROBOPT_MeasureEvaluation_requiresOptimizationDomain_doc
"Whether rebuilding the measure requires an explicit optimization domain.

Returns
-------
required : bool
    True when the measure optimizes over its distribution support and
    therefore needs a domain when its distribution is replaced."
%enddef
%feature("docstring") OTROBOPT::MeasureEvaluationImplementation::requiresOptimizationDomain
OTROBOPT_MeasureEvaluation_requiresOptimizationDomain_doc
// ---------------------------------------------------------------------

%define OTROBOPT_MeasureEvaluation_setIntegrationAlgorithm_doc
"Integration algorithm accessor.

Parameters
----------
algorithm : :py:class:`openturns.IntegrationAlgorithm`
    Algorithm used to compute the exact value of the measure."
%enddef
%feature("docstring") OTROBOPT::MeasureEvaluationImplementation::setIntegrationAlgorithm
OTROBOPT_MeasureEvaluation_setIntegrationAlgorithm_doc

// ---------------------------------------------------------------------

%define OTROBOPT_MeasureEvaluation_getIntegrationAlgorithm_doc
"Integration algorithm accessor.

Returns
-------
algorithm : :py:class:`openturns.IntegrationAlgorithm`
    Algorithm used to compute the exact value of the measure."
%enddef
%feature("docstring") OTROBOPT::MeasureEvaluationImplementation::getIntegrationAlgorithm
OTROBOPT_MeasureEvaluation_getIntegrationAlgorithm_doc

// ---------------------------------------------------------------------

%define OTROBOPT_MeasureEvaluation_getPDFThreshold_doc
"PDF threshold accessor.

Returns
-------
threshold : float
    PDF threshold under which the contribution is not computed."
%enddef
%feature("docstring") OTROBOPT::MeasureEvaluationImplementation::getPDFThreshold
OTROBOPT_MeasureEvaluation_getPDFThreshold_doc

// ---------------------------------------------------------------------

%define OTROBOPT_MeasureEvaluation_setPDFThreshold_doc
"PDF threshold accessor.

Parameters
----------
threshold : float
    PDF threshold under which the contribution is not computed."
%enddef
%feature("docstring") OTROBOPT::MeasureEvaluationImplementation::setPDFThreshold
OTROBOPT_MeasureEvaluation_setPDFThreshold_doc

// ---------------------------------------------------------------------
