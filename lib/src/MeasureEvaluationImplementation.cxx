//                                               -*- C++ -*-
/**
 *  @brief Measure function evaluation implementation
 *
 *  Copyright 2005-2026 Airbus-EDF-IMACS-ONERA-Phimeca
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "otrobopt/MeasureEvaluationImplementation.hxx"
#include "otrobopt/MeasureEvaluation.hxx"
#include <openturns/Exception.hxx>
#include <openturns/NoGradient.hxx>
#include <openturns/ParametricEvaluation.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/IteratedQuadrature.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(MeasureEvaluationImplementation)

static Factory<MeasureEvaluationImplementation> Factory_MeasureEvaluationImplementation;

namespace
{

/* Assemble the full input point of a parametric evaluation from the input
   point x and the parameter theta. */
Point buildFullPoint(const Point & x,
                     const Point & theta,
                     const Indices & inputPositions,
                     const Indices & parametersPositions,
                     const UnsignedInteger fullDimension)
{
  Point fullPoint(fullDimension);
  for (UnsignedInteger i = 0; i < parametersPositions.getSize(); ++i)
    fullPoint[parametersPositions[i]] = theta[i];
  for (UnsignedInteger i = 0; i < inputPositions.getSize(); ++i)
    fullPoint[inputPositions[i]] = x[i];
  return fullPoint;
}

}


/* Default constructor */
MeasureEvaluationImplementation::MeasureEvaluationImplementation()
  : EvaluationImplementation()
  , pdfThreshold_(SpecFunc::Precision)
{
  // Nothing to do
}

MeasureEvaluationImplementation::MeasureEvaluationImplementation (const Function & function,
    const Distribution & distribution)
  : EvaluationImplementation()
  , function_(function)
  , distribution_(distribution)
  , integrationAlgorithm_(IteratedQuadrature())
  , pdfThreshold_(SpecFunc::Precision)
{
  if (distribution.getDimension() != function.getParameter().getDimension())
    throw InvalidDimensionException(HERE) << "Function parameter dimension (" << function.getParameter().getDimension()
                                          << ") must match the distribution dimension (" << distribution.getDimension() << ")";
  setInputDescription(function_.getInputDescription());
  setOutputDescription(function_.getOutputDescription());
}


/* Virtual constructor method */
MeasureEvaluationImplementation * MeasureEvaluationImplementation::clone() const
{
  return new MeasureEvaluationImplementation(*this);
}


/* Evaluation */
Point MeasureEvaluationImplementation::operator()(const Point & /*inP*/) const
{
  throw NotYetImplementedException(HERE) << "MeasureEvaluationImplementation::operator()(const Point & inP)";
}


/* String converter */
String MeasureEvaluationImplementation::__repr__() const
{
  OSS oss;
  oss << "class=" << MeasureEvaluationImplementation::GetClassName()
      << " distribution=" << distribution_
      << " function=" << function_;
  return oss;
}

String MeasureEvaluationImplementation::__str__(const String & offset) const
{
  OSS oss;
  oss << offset << __repr__();
  return oss;
}


/* Distribution accessor */
void MeasureEvaluationImplementation::setDistribution(const Distribution & distribution)
{
  distribution_ = distribution;
}


Distribution MeasureEvaluationImplementation::getDistribution() const
{
  return distribution_;
}


/* Function accessor */
void MeasureEvaluationImplementation::setFunction(const Function & function)
{
  function_ = function;
}

Function MeasureEvaluationImplementation::getFunction() const
{
  return function_;
}


/* Rebuild the measure over a new function and distribution */
MeasureEvaluation MeasureEvaluationImplementation::rebuild(const Function & function,
                                                           const Distribution & distribution) const
{
  return rebuild(function, distribution, Interval());
}

MeasureEvaluation MeasureEvaluationImplementation::rebuild(const Function & /*function*/,
                                                           const Distribution & /*distribution*/,
                                                           const Interval & /*optimizationDomain*/) const
{
  throw NotYetImplementedException(HERE) << "MeasureEvaluationImplementation::rebuild is not implemented for "
                                         << getClassName();
}

Bool MeasureEvaluationImplementation::requiresOptimizationDomain() const
{
  return false;
}


/* Build the analytic gradient of the measure, if available */
Gradient MeasureEvaluationImplementation::buildGradient() const
{
  // No analytic gradient by default: MeasureFunction installs a centered
  // finite difference gradient over the measure evaluation. NoGradient is the
  // OpenTURNS marker for an undefined gradient.
  return Gradient(NoGradient());
}

/* Derivative of the integrand with respect to the measure input */
Matrix MeasureEvaluationImplementation::integrandGradient(const Point & /*x*/,
                                                          const Point & /*theta*/,
                                                          const Point & /*auxiliaryValues*/) const
{
  throw NotYetImplementedException(HERE) << "MeasureEvaluationImplementation::integrandGradient is not implemented for "
                                         << getClassName();
}

/* Auxiliary values required by integrandGradient */
Point MeasureEvaluationImplementation::computeAuxiliaryValues(const Point & /*x*/) const
{
  return Point();
}

/* Evaluate the underlying function at x for the parameter theta */
Point MeasureEvaluationImplementation::functionValue(const Point & x, const Point & theta) const
{
  // A parametric evaluation can be evaluated on the full point (x, theta),
  // which avoids changing the state of the underlying function.
  const Evaluation evaluation(function_.getEvaluation());
  const ParametricEvaluation * p_parametricEvaluation = dynamic_cast<const ParametricEvaluation *>(evaluation.getImplementation().get());
  if (p_parametricEvaluation)
  {
    const Function fullFunction(p_parametricEvaluation->getFunction());
    return fullFunction(buildFullPoint(x, theta,
                                       p_parametricEvaluation->getInputPositions(),
                                       p_parametricEvaluation->getParametersPositions(),
                                       fullFunction.getInputDimension()));
  }
  Function function(function_);
  function.setParameter(theta);
  return function(x);
}

/* Gradient of the underlying function at x for the parameter theta */
Matrix MeasureEvaluationImplementation::functionGradient(const Point & x, const Point & theta) const
{
  // Differentiate the full function at the point (x, theta) and keep the rows
  // associated with the input coordinates. This gives the exact derivative
  // even for OpenTURNS versions where setParameter is not propagated to the
  // gradient of a ParametricFunction.
  const Evaluation evaluation(function_.getEvaluation());
  const ParametricEvaluation * p_parametricEvaluation = dynamic_cast<const ParametricEvaluation *>(evaluation.getImplementation().get());
  if (p_parametricEvaluation)
  {
    const Function fullFunction(p_parametricEvaluation->getFunction());
    const Indices inputPositions(p_parametricEvaluation->getInputPositions());
    const Matrix fullGradient(fullFunction.gradient(buildFullPoint(x, theta,
        inputPositions,
        p_parametricEvaluation->getParametersPositions(),
        fullFunction.getInputDimension())));
    const UnsignedInteger inputDimension = x.getDimension();
    const UnsignedInteger outputDimension = fullFunction.getOutputDimension();
    Matrix outG(inputDimension, outputDimension);
    for (UnsignedInteger i = 0; i < inputDimension; ++i)
      for (UnsignedInteger j = 0; j < outputDimension; ++j)
        outG(i, j) = fullGradient(inputPositions[i], j);
    return outG;
  }
  Function function(function_);
  function.setParameter(theta);
  return function.gradient(x);
}


UnsignedInteger MeasureEvaluationImplementation::getInputDimension() const
{
  return function_.getInputDimension();
}

UnsignedInteger MeasureEvaluationImplementation::getOutputDimension() const
{
  return function_.getOutputDimension();
}

/* Integration algorithm$ accessor */
void MeasureEvaluationImplementation::setIntegrationAlgorithm(const IntegrationAlgorithm & algorithm)
{
  integrationAlgorithm_ = algorithm;
}

IntegrationAlgorithm MeasureEvaluationImplementation::getIntegrationAlgorithm() const
{
  return integrationAlgorithm_;
}


/* PDF threshold accessor */
Scalar MeasureEvaluationImplementation::getPDFThreshold() const
{
  return pdfThreshold_;
}

void MeasureEvaluationImplementation::setPDFThreshold(const Scalar threshold)
{
  pdfThreshold_ = threshold;
}


/* Method save() stores the object through the StorageManager */
void MeasureEvaluationImplementation::save(Advocate & adv) const
{
  EvaluationImplementation::save(adv);
  adv.saveAttribute("distribution_", distribution_);
  adv.saveAttribute("function_", function_);
  adv.saveAttribute("integrationAlgorithm_", integrationAlgorithm_);
  adv.saveAttribute("pdfThreshold_", pdfThreshold_);
}

/* Method load() reloads the object from the StorageManager */
void MeasureEvaluationImplementation::load(Advocate & adv)
{
  EvaluationImplementation::load(adv);
  adv.loadAttribute("distribution_", distribution_);
  adv.loadAttribute("function_", function_);
  adv.loadAttribute("integrationAlgorithm_", integrationAlgorithm_);
  adv.loadAttribute("pdfThreshold_", pdfThreshold_);
}

} /* namespace OTROBOPT */
