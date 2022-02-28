//                                               -*- C++ -*-
/**
 *  @brief Joint probability measure
 *
 *  Copyright 2005-2018 Airbus-EDF-IMACS-Phimeca
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
#include "otrobopt/JointChanceMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/IteratedQuadrature.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(JointChanceMeasure)

static Factory<JointChanceMeasure> Factory_JointChanceMeasure;


/* Default constructor */
JointChanceMeasure::JointChanceMeasure()
  : MeasureEvaluationImplementation()
  , alpha_(0.0)
{
  // Set the default integration algorithm
  GaussKronrod gkr;
  gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("JointChanceMeasure-GaussKronrodRule")));
  integrationAlgorithm_ = IteratedQuadrature(gkr);
}

/* Parameter constructor */
JointChanceMeasure::JointChanceMeasure (const Function & function,
                                        const Distribution & distribution,
                                        const ComparisonOperator & op,
                                        const Scalar alpha)
  : MeasureEvaluationImplementation(function, distribution)
  , operator_(op)
  , alpha_(0.0)
{
  setAlpha(alpha);
  setOutputDescription(Description(1, "P"));
  // Set the default integration algorithm
  GaussKronrod gkr;
  gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("JointChanceMeasure-GaussKronrodRule")));
  integrationAlgorithm_ = IteratedQuadrature(gkr);
}

/* Virtual constructor method */
JointChanceMeasure * JointChanceMeasure::clone() const
{
  return new JointChanceMeasure(*this);
}


class JointChanceMeasureParametricFunctionWrapper : public FunctionImplementation
{
public:
  JointChanceMeasureParametricFunctionWrapper(const Point & x,
      const Function & function,
      const Distribution & distribution,
      const Scalar pdfThreshold)
    : FunctionImplementation()
    , x_(x)
    , function_(function)
    , distribution_(distribution)
    , pdfThreshold_(pdfThreshold)
  {
    // Nothing to do
  }

  virtual JointChanceMeasureParametricFunctionWrapper * clone() const
  {
    return new JointChanceMeasureParametricFunctionWrapper(*this);
  }

  Point operator()(const Point & theta) const
  {
    const Scalar pdf = distribution_.computePDF(theta);
    if (pdf <= pdfThreshold_) return Point(1, 0.0);
    Function function(function_);
    function.setParameter(theta);
    const Point y(function(x_));
    const UnsignedInteger outputDimension = y.getDimension();
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
      if (y[j] < 0.0) return Point(1, 0.0);
    return Point(1, pdf);
  }

  Sample operator()(const Sample & theta) const
  {
    const UnsignedInteger size = theta.getSize();
    Sample outS(size, 1);
    for (UnsignedInteger i = 0; i < size; ++ i)
      outS(i, 0) = operator()(theta[i])[0];
    return outS;
  }

  UnsignedInteger getInputDimension() const
  {
    return function_.getParameterDimension();
  }

  UnsignedInteger getOutputDimension() const
  {
    return 1;
  }

  Description getInputDescription() const
  {
    return function_.getParameterDescription();
  }

  Description getOutputDescription() const
  {
    return Description(1, "P");
  }

protected:
  Point x_;
  Function function_;
  Distribution distribution_;
  Scalar pdfThreshold_;
};


/* Evaluation */
Point JointChanceMeasure::operator()(const Point & inP) const
{
  Function function(getFunction());
  const UnsignedInteger outputDimension = function.getOutputDimension();
  Point outP(1);
  if (getDistribution().isContinuous())
  {
    Pointer<FunctionImplementation> p_wrapper(new JointChanceMeasureParametricFunctionWrapper(inP, function, getDistribution(), pdfThreshold_));
    const Function G(p_wrapper);
    outP = integrationAlgorithm_.integrate(G, getDistribution().getRange());
  }
  else
  {
    const Point pdfs(getDistribution().getProbabilities());
    const Sample parameters(getDistribution().getSupport());
    const UnsignedInteger size = parameters.getSize();
    Sample values(0, outputDimension);
    Point weights(0);
    for (UnsignedInteger i = 0; i < size; ++i)
    {
      if (pdfs[i] > pdfThreshold_)
        {
          function.setParameter(parameters[i]);
          values.add(function(inP));
          weights.add(pdfs[i]);
        }
    }
    // Here we compute the marginal complementary CDF locally to avoid
    // the creation cost of the UserDefined distributions
    for (UnsignedInteger i = 0; i < weights.getSize(); ++ i)
    {
      Bool allOk = true;
      for (UnsignedInteger j = 0; j < outputDimension; ++ j)
        if (values(i, j) < 0.0)
        {
          allOk = false;
          break;
        }
      if (allOk) outP[0] += weights[i];
    } // for i
  }
  outP[0] = operator_.operator()(1.0, 2.0) ? alpha_ - outP[0] : outP[0] - alpha_;
  return outP;
}

UnsignedInteger JointChanceMeasure::getOutputDimension() const
{
  return 1;
}

/* Alpha coefficient accessor */
void JointChanceMeasure::setAlpha(const Scalar alpha)
{
  if (!(alpha >= 0.0) || !(alpha <= 1.0))
    throw InvalidArgumentException(HERE) << "Alpha should be in (0, 1)";
  alpha_ = alpha;
}

Scalar JointChanceMeasure::getAlpha() const
{
  return alpha_;
}

/* String converter */
String JointChanceMeasure::__repr__() const
{
  OSS oss;
  oss << "class=" << JointChanceMeasure::GetClassName()
      << " alpha=" << alpha_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void JointChanceMeasure::save(Advocate & adv) const
{
  MeasureEvaluationImplementation::save(adv);
  adv.saveAttribute("alpha_", alpha_);
  adv.saveAttribute("operator_", operator_);
}

/* Method load() reloads the object from the StorageManager */
void JointChanceMeasure::load(Advocate & adv)
{
  MeasureEvaluationImplementation::load(adv);
  adv.loadAttribute("alpha_", alpha_);
  adv.loadAttribute("operator_", operator_);
}


} /* namespace OTROBOPT */
