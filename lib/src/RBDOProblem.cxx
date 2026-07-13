//                                               -*- C++ -*-
/**
 *  @brief Reliability-Based Design Optimization problem
 *
 *  Copyright 2005-2024 Airbus-EDF-IMACS-Phimeca
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

#include "otrobopt/RBDOProblem.hxx"
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/BlockIndependentDistribution.hxx>
#include <openturns/ParametricFunction.hxx>
#include <openturns/SymbolicFunction.hxx>
#include <openturns/ComposedFunction.hxx>
#include <openturns/Less.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(RBDOProblem)

static Factory<RBDOProblem> Factory_RBDOProblem;

/* Default constructor */
RBDOProblem::RBDOProblem()
  : RobustOptimizationProblem()
  , constraintOperator_(Less())
{
  // Nothing to do
}

/* Parameter constructor */
RBDOProblem::RBDOProblem(const Function & objective,
                         const Function & constraintFunction,
                         const Distribution & distributionX,
                         const Distribution & distributionZ,
                         const Scalar targetBeta,
                         const ComparisonOperator & constraintOperator,
                         const Indices & activeDesignIndices,
                         const Indices & activeParameterIndices)
  : RobustOptimizationProblem()
  , probabilisticConstraintFunction_(constraintFunction)
  , distributionX_(distributionX)
  , distributionZ_(distributionZ)
  , targetBeta_(targetBeta)
  , constraintOperator_(constraintOperator)
  , activeDesignIndices_(activeDesignIndices)
  , activeParameterIndices_(activeParameterIndices)
  , lenD_(activeDesignIndices.getSize())
  , lenP_(activeParameterIndices.getSize())
{
  setObjective(objective);
  setMinimization(true);

  // Build reliability measure for SequentialMonteCarlo and direct solvers
  const UnsignedInteger lenDP = lenD_ + lenP_;
  const UnsignedInteger nX = distributionX.getDimension();

  if (nX > 0)
  {
    // Detect Z from constraint function signature: g(d, p, x, z)
    // hasZ_ stays false for examples that pass Distribution() as "no Z"
    const UnsignedInteger gDim = constraintFunction.getInputDimension();
    if (gDim > lenDP + nX)
    {
      const UnsignedInteger nZ = gDim - lenDP - nX;
      // Validate distributionZ dimension when Z is present
      if (distributionZ_.getDimension() != nZ)
        throw InvalidArgumentException(HERE)
            << "DistributionZ dimension mismatch: expected " << nZ
            << " but got " << distributionZ_.getDimension();
      setDistributionZ(distributionZ_);
    }

    // Convert X inputs to parameters for the JointChanceMeasure
    Indices xzParamIndices;
    if (hasZ_)
    {
      const UnsignedInteger nZ = distributionZ_.getDimension();
      for (UnsignedInteger i = lenDP; i < lenDP + nX + nZ; ++i)
        xzParamIndices.add(i);
    }
    else
    {
      for (UnsignedInteger i = lenDP; i < lenDP + nX; ++i)
        xzParamIndices.add(i);
    }
    const ParametricFunction gForMeasure(constraintFunction, xzParamIndices, Point(xzParamIndices.getSize(), 0.0));

    // Joint distribution of X (and Z if explicitly set)
    const Scalar targetProbability = DistFunc::pNormal(targetBeta);
    if (hasZ_)
    {
      const Collection<Distribution> marginals({distributionX_, distributionZ_});
      const Distribution jointDist = BlockIndependentDistribution(marginals);
      const JointChanceMeasure reliabilityMeasure(gForMeasure, jointDist, constraintOperator, targetProbability);
      setReliabilityMeasure(reliabilityMeasure);
    }
    else
    {
      const JointChanceMeasure reliabilityMeasure(gForMeasure, distributionX_, constraintOperator, targetProbability);
      setReliabilityMeasure(reliabilityMeasure);
    }

    // JointChanceMeasure with Less() returns alpha - P[g >= 0].
    // For Cobyla (constraint >= 0 convention) we need P[g >= 0] - alpha >= 0,
    // which is the correct RBDO constraint: survival probability >= target.
    // Negate the result for Less to match the expected sign convention.
    if (constraintOperator.getImplementation()->getClassName() == "Less")
    {
      const Function rawConstraint(getInequalityConstraint());
      const SymbolicFunction negateFunc("x", "-x");
      const Function correctedConstraint(ComposedFunction(negateFunc, rawConstraint));
      setInequalityConstraint(correctedConstraint);
    }
  }
}

/* Virtual constructor */
RBDOProblem * RBDOProblem::clone() const
{
  return new RBDOProblem(*this);
}

OT::Distribution RBDOProblem::buildJointDistribution() const
{
  if (!hasZ_)
    return distributionX_;
  return BlockIndependentDistribution(OT::Collection<OT::Distribution>({distributionX_, distributionZ_}));
}

/* Constraint function accessors */
void RBDOProblem::setConstraintFunction(const Function & function)
{
  probabilisticConstraintFunction_ = function;
}

Function RBDOProblem::getConstraintFunction() const
{
  return probabilisticConstraintFunction_;
}

/* Distribution X accessors */
void RBDOProblem::setDistributionX(const Distribution & distribution)
{
  distributionX_ = distribution;
}

Distribution RBDOProblem::getDistributionX() const
{
  return distributionX_;
}

/* Distribution Z accessors */
void RBDOProblem::setDistributionZ(const Distribution & distribution)
{
  distributionZ_ = distribution;
  hasZ_ = true;
}

Distribution RBDOProblem::getDistributionZ() const
{
  return distributionZ_;
}

Bool RBDOProblem::hasDistributionZ() const
{
  return hasZ_;
}

/* Target beta accessors */
void RBDOProblem::setTargetBeta(Scalar targetBeta)
{
  targetBeta_ = targetBeta;
}

Scalar RBDOProblem::getTargetBeta() const
{
  return targetBeta_;
}

/* Constraint operator accessors */
void RBDOProblem::setConstraintOperator(const ComparisonOperator & op)
{
  constraintOperator_ = op;
}

ComparisonOperator RBDOProblem::getConstraintOperator() const
{
  return constraintOperator_;
}

/* Active design indices accessors */
void RBDOProblem::setActiveDesignIndices(const Indices & indices)
{
  activeDesignIndices_ = indices;
}

Indices RBDOProblem::getActiveDesignIndices() const
{
  return activeDesignIndices_;
}

/* Active parameter indices accessors */
void RBDOProblem::setActiveParameterIndices(const Indices & indices)
{
  activeParameterIndices_ = indices;
}

Indices RBDOProblem::getActiveParameterIndices() const
{
  return activeParameterIndices_;
}

/* Design dimension accessors */
void RBDOProblem::setDesignDimension(UnsignedInteger lenD)
{
  lenD_ = lenD;
}

UnsignedInteger RBDOProblem::getDesignDimension() const
{
  return lenD_;
}

/* Parameter dimension accessors */
void RBDOProblem::setParameterDimension(UnsignedInteger lenP)
{
  lenP_ = lenP;
}

UnsignedInteger RBDOProblem::getParameterDimension() const
{
  return lenP_;
}

/* String converter */
String RBDOProblem::__repr__() const
{
  OSS oss;
  oss << "class=" << RBDOProblem::GetClassName()
      << " objective=" << getObjective()
      << " targetBeta=" << targetBeta_;
  return oss;
}

/* Method save */
void RBDOProblem::save(Advocate & adv) const
{
  RobustOptimizationProblem::save(adv);
  adv.saveAttribute("probabilisticConstraintFunction_", probabilisticConstraintFunction_);
  adv.saveAttribute("distributionX_", distributionX_);
  adv.saveAttribute("distributionZ_", distributionZ_);
  adv.saveAttribute("hasZ_", hasZ_);
  adv.saveAttribute("targetBeta_", targetBeta_);
  adv.saveAttribute("constraintOperator_", constraintOperator_);
  adv.saveAttribute("activeDesignIndices_", activeDesignIndices_);
  adv.saveAttribute("activeParameterIndices_", activeParameterIndices_);
  adv.saveAttribute("lenD_", lenD_);
  adv.saveAttribute("lenP_", lenP_);
}

/* Method load */
void RBDOProblem::load(Advocate & adv)
{
  RobustOptimizationProblem::load(adv);
  adv.loadAttribute("probabilisticConstraintFunction_", probabilisticConstraintFunction_);
  adv.loadAttribute("distributionX_", distributionX_);
  adv.loadAttribute("distributionZ_", distributionZ_);
  adv.loadAttribute("hasZ_", hasZ_);
  adv.loadAttribute("targetBeta_", targetBeta_);
  adv.loadAttribute("constraintOperator_", constraintOperator_);
  adv.loadAttribute("activeDesignIndices_", activeDesignIndices_);
  adv.loadAttribute("activeParameterIndices_", activeParameterIndices_);
  adv.loadAttribute("lenD_", lenD_);
  adv.loadAttribute("lenP_", lenP_);
}

} /* namespace OTROBOPT */
