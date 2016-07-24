//                                               -*- C++ -*-
/**
 *  @brief Joint probability measure
 *
 *  Copyright 2005-2016 Airbus-EDF-IMACS-Phimeca
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

CLASSNAMEINIT(JointChanceMeasure);

static Factory<JointChanceMeasure> Factory_JointChanceMeasure;


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
  {
    // Nothing to do
  }

  virtual JointChanceMeasureParametricFunctionWrapper * clone() const
  {
    return new JointChanceMeasureParametricFunctionWrapper(*this);
  }

  NumericalPoint operator()(const NumericalPoint & theta) const
  {
    NumericalMathFunction function(function_);
    const NumericalPoint y(function(x_, theta));
    const UnsignedInteger outputDimension = y.getDimension();
    for (UnsignedInteger j = 0; j < outputDimension; ++ j)
      if (y[j] < 0.0) return NumericalPoint(1, 0.0);
    return NumericalPoint(1, distribution_.computePDF(theta));
  }

  NumericalSample operator()(const NumericalSample & theta) const
  {
    NumericalMathFunction function(function_);
    // To benefit from possible parallelism
    const NumericalSample y(function(x_, theta));
    const UnsignedInteger size = y.getSize();
    const UnsignedInteger outputDimension = y.getDimension();
    // First pass to select the points at which we have to compute the
    // PDF as it can be costly for some distributions
    NumericalSample activeTheta(0, theta.getDimension());
    Indices activeIndices(0);
    for (UnsignedInteger i = 0; i < size; ++i)
      {
	Bool allOk(true);
	for (UnsignedInteger j = 0; j < outputDimension; ++ j)
	  if (y[i][j] < 0.0)
	    {
	      allOk = false;
	      break;
	    } // y[i][j] < 0.0
	if (allOk)
	  {
	    activeTheta.add(theta[i]);
	    activeIndices.add(i);
	  } // allOk
      } // for i
    // Exploit possible parallelization of computePDF
    const NumericalSample pdf(distribution_.computePDF(activeTheta));
    NumericalSample outS(size, 1);
    for (UnsignedInteger i = 0; i < activeTheta.getSize(); ++i)
      outS[activeIndices[i]][0] = pdf[i][0];
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
    gkr.setRule(static_cast<GaussKronrodRule::GaussKronrodPair>(ResourceMap::GetAsUnsignedInteger("JointChanceMeasure-GaussKronrodRule")));
    const IteratedQuadrature algo(gkr);
    Pointer<NumericalMathFunctionImplementation> p_wrapper(new JointChanceMeasureParametricFunctionWrapper(inP, function, getDistribution()));
    const NumericalMathFunction G(p_wrapper);
    outP = algo.integrate(G, getDistribution().getRange());
  }
  else
  {
    // To benefit from possible parallelization
    const NumericalSample values(function(inP, getDistribution().getSupport()));
    const NumericalPoint weights(getDistribution().getProbabilities());
    // Here we compute the marginal complementary CDF locally to avoid
    // the creation cost of the UserDefined distributions
    const UnsignedInteger size = values.getSize();
    for (UnsignedInteger i = 0; i < size; ++ i)
    {
      Bool allOk = true;
      for (UnsignedInteger j = 0; j < outputDimension; ++ j)
        if (values[i][j] < 0.0)
          {
            allOk = false;
            break;
          }
      if (allOk) outP[0] += weights[i];
    } // for i
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
