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
#ifndef OTROBOPT_RBDOPROBLEM_HXX
#define OTROBOPT_RBDOPROBLEM_HXX

#include <openturns/Function.hxx>
#include <openturns/Point.hxx>
#include <openturns/Distribution.hxx>
#include <openturns/ComparisonOperator.hxx>
#include <openturns/Interval.hxx>
#include <openturns/PersistentObject.hxx>
#include <openturns/Indices.hxx>
#include <openturns/DistFunc.hxx>

#include "otrobopt/OTRobOptprivate.hxx"
#include "otrobopt/RobustOptimizationProblem.hxx"
#include "otrobopt/JointChanceMeasure.hxx"

namespace OTROBOPT
{

/**
 * @class RBDOProblem
 *
 * Defines a Reliability-Based Design Optimization problem.
 * Stores the objective function, probabilistic and deterministic constraints,
 * bounds, and the structure of the design/parameter vector [d, p].
 */
class OTROBOPT_API RBDOProblem
  : public RobustOptimizationProblem
{
  CLASSNAME

public:

  /** Default constructor */
  RBDOProblem();

  /** Parameter constructor */
  RBDOProblem(const OT::Function & objective,
              const OT::Function & constraintFunction,
              const OT::Distribution & distributionX,
              const OT::Distribution & distributionZ,
              const OT::Scalar targetBeta,
              const OT::ComparisonOperator & constraintOperator,
              const OT::Indices & activeDesignIndices,
              const OT::Indices & activeParameterIndices);

  /** Virtual constructor */
  RBDOProblem * clone() const override;

  /** Probabilistic constraint function (g(d, X, Z)) */
  void setConstraintFunction(const OT::Function & function);
  OT::Function getConstraintFunction() const;

  /** Distribution of X (random variables that may depend on design) */
  void setDistributionX(const OT::Distribution & distribution);
  OT::Distribution getDistributionX() const;

  /** Distribution of Z (fixed random variables) */
  void setDistributionZ(const OT::Distribution & distribution);
  OT::Distribution getDistributionZ() const;
  OT::Bool hasDistributionZ() const;

  /** Target reliability index */
  void setTargetBeta(OT::Scalar targetBeta);
  OT::Scalar getTargetBeta() const;

  /** Comparison operator for failure event (Less: P[g<=0], Greater: P[g>=0]) */
  void setConstraintOperator(const OT::ComparisonOperator & op);
  OT::ComparisonOperator getConstraintOperator() const;

  /** Indices of design variables d in the combined vector [d, p] */
  void setActiveDesignIndices(const OT::Indices & indices);
  OT::Indices getActiveDesignIndices() const;

  /** Indices of parameters p in the combined vector [d, p] */
  void setActiveParameterIndices(const OT::Indices & indices);
  OT::Indices getActiveParameterIndices() const;

  /** Dimension of d */
  void setDesignDimension(OT::UnsignedInteger lenD);
  OT::UnsignedInteger getDesignDimension() const;

  /** Dimension of p */
  void setParameterDimension(OT::UnsignedInteger lenP);
  OT::UnsignedInteger getParameterDimension() const;

  /** Build the joint distribution of (X, Z) */
  OT::Distribution buildJointDistribution() const;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

private:
  OT::Function probabilisticConstraintFunction_;
  OT::Distribution distributionX_;
  OT::Distribution distributionZ_;
  OT::Scalar targetBeta_ = 3.0;
  OT::ComparisonOperator constraintOperator_;
  OT::Indices activeDesignIndices_;
  OT::Indices activeParameterIndices_;
  OT::UnsignedInteger lenD_ = 0;
  OT::UnsignedInteger lenP_ = 0;
  OT::Bool hasZ_ = false;

}; /* class RBDOProblem */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_RBDOPROBLEM_HXX */
