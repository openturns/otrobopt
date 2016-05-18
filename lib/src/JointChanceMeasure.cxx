//                                               -*- C++ -*-
/**
 *  @brief Joint probability measure
 *
 *  Copyright 2005-2016 Airbus-EDF-IMACS-Phimeca
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
#include "otrobopt/JointChanceMeasure.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/GaussKronrod.hxx>
#include <openturns/IteratedQuadrature.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(JointChanceMeasure);

static Factory<JointChanceMeasure> RegisteredFactory;


/* Default constructor */
JointChanceMeasure::JointChanceMeasure()
  : MeasureEvaluationImplementation()
  , alpha_(0.0)
{
  // Nothing to do
}

/* Parameter constructor */
JointChanceMeasure::JointChanceMeasure (const NumericalMathFunction & function,
                                        const Distribution & distribution,
                                        const ComparisonOperator & op,
                                        const NumericalScalar alpha)
  : MeasureEvaluationImplementation(function, distribution)
  , operator_(op)
  , alpha_(0.0)
{
  setAlpha(alpha);
  setOutputDescription(Description(1, "P"));
}

/* Virtual constructor method */
JointChanceMeasure * JointChanceMeasure::clone() const
{
  return new JointChanceMeasure(*this);
}


class JointChanceMeasureParametricFunctionWrapper : public NumericalMathFunctionImplementation
{
public:
  JointChanceMeasureParametricFunctionWrapper(const NumericalPoint & x,
                                              const NumericalMathFunction & function,
                                              const Distribution & distribution)
  : NumericalMathFunctionImplementation()
  , x_(x)
  , function_(function)
  , distribution_(distribution)
  {}

  virtual JointChanceMeasureParametricFunctionWrapper * clone() const
  {
    return new JointChanceMeasureParametricFunctionWrapper(*this);
  }

  NumericalPoint operator()(const NumericalPoint & theta) const
  {
    const UnsignedInteger outputDimension = function_.getOutputDimension();
    NumericalMathFunction function(function_);
    const NumericalPoint y(function(x_, theta));
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
      if (y[j] < 0.0) return NumericalPoint(1, 0.0);
    return NumericalPoint(1, distribution_.computePDF(theta));
  }

  NumericalSample operator()(const NumericalSample & theta) const
  {
    const UnsignedInteger size = theta.getSize();
    NumericalSample outS(size, function_.getOutputDimension());
    for (UnsignedInteger i = 0; i < size; ++ i)
    {
      outS[i] = operator()(theta[i]);
    }
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
  NumericalPoint x_;
  NumericalMathFunction function_;
  Distribution distribution_;
};


/* Evaluation */
NumericalPoint JointChanceMeasure::operator()(const NumericalPoint & inP) const
{
  NumericalMathFunction function(getFunction());
  NumericalPoint parameter(function.getParameter());
  const UnsignedInteger outputDimension = getFunction().getOutputDimension();
  NumericalPoint outP(1);
  if (getDistribution().isContinuous())
  {
    GaussKronrod gkr;
    gkr.setRule(static_cast<OT::GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("JointChanceMeasure-GaussKronrodRule")));
    const IteratedQuadrature algo(gkr);
    Pointer<NumericalMathFunctionImplementation> p_wrapper(new JointChanceMeasureParametricFunctionWrapper(inP, function, getDistribution()));
    const NumericalMathFunction G(p_wrapper);
    outP = algo.integrate(G, getDistribution().getRange());
  }
  else
  {
    NumericalSample support(getDistribution().getSupport());
    const UnsignedInteger size = support.getSize();
    for (UnsignedInteger i = 0; i < size; ++ i)
    {
      const NumericalPoint outPi(function(inP, support[i]));
      Bool allOk = true;
      for (UnsignedInteger j = 0; j < outputDimension; ++ j)
        if (outPi[j] < 0.0)
          {
            allOk = false;
            break;
          }
      if (allOk) outP[0] += 1.0;
    }
    outP[0] /= size;
  }
  outP[0] = operator_.operator()(1.0, 2.0) ? alpha_ - outP[0] : outP[0] - alpha_;
  function.setParameter(parameter);
  return outP;
}

UnsignedInteger JointChanceMeasure::getOutputDimension() const
{
  return 1;
}

/* Alpha coefficient accessor */
void JointChanceMeasure::setAlpha(const NumericalScalar alpha)
{
  if (!(alpha >= 0.0) || !(alpha <= 1.0))
    throw InvalidArgumentException(HERE) << "Alpha should be in (0, 1)";
  alpha_ = alpha;
}

NumericalScalar JointChanceMeasure::getAlpha() const
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
