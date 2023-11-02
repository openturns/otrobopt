//                                               -*- C++ -*-
/**
 *  @brief InverseFORM
 *
 *  Copyright 2005-2023 Airbus-EDF-IMACS-Phimeca
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
#ifndef OTROBOPT_INVERSEFORM_HXX
#define OTROBOPT_INVERSEFORM_HXX

#include "otrobopt/InverseFORMResult.hxx"

#include <openturns/PersistentObject.hxx>
#include <openturns/RandomVector.hxx>

namespace OTROBOPT {

/**
 * @class InverseFORM
 * InverseFORM implements the First Order Reliability Method
 * and compute the results of such kind of analyses
 */

class OTROBOPT_API InverseFORM
  : public OT::PersistentObject
{

  CLASSNAME
public:

  /** Default constructor */
  InverseFORM();

  /** Constructor with parameters */
  InverseFORM(const OT::RandomVector & event,
              const OT::String & parameterName,
              const OT::Point & physicalStartingPoint);

  /** Virtual constructor */
  virtual InverseFORM * clone() const override;

  void setTargetBeta(OT::Scalar targetBeta);
  OT::Scalar getTargetBeta() const;

  void setFixedStep(OT::Bool fixedStep);
  OT::Bool getFixedStep() const;

  void setFixedStepValue(OT::Scalar fixedStepValue);
  OT::Scalar getFixedStepValue() const;

  void setVariableStepMaxIterations(OT::UnsignedInteger variableStepMaxIterations);
  OT::UnsignedInteger getVariableStepMaxIterations() const;

  void setMaximumIteration(OT::UnsignedInteger maximumIteration);
  OT::UnsignedInteger getMaximumIteration() const;

  void setVariableConvergence(OT::Scalar variableConvergence);
  OT::Scalar getVariableConvergence() const;

  void setBetaConvergence(OT::Scalar betaConvergence);
  OT::Scalar getBetaConvergence() const;

  void setLimitStateConvergence(OT::Scalar limitStateConvergence);
  OT::Scalar getLimitStateConvergence() const;

  /** Result accessor */
  void setResult(const InverseFORMResult & result);
  InverseFORMResult getResult() const;

  /** String converter */
  OT::String __repr__() const override;

  /** Function that computes the design point by re-using the Analytical::run() and creates a InverseFORM::Result */
  void run();

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

private:
  OT::Scalar meritFunction(const OT::Point & u, const OT::Scalar levelValue) const;
  OT::Function getG(const OT::Scalar p);

  
  OT::RandomVector event_;
  OT::String parameterName_;
  OT::UnsignedInteger parameterIndex_ = 0;
  OT::Point physicalStartingPoint_;

  InverseFORMResult result_;
  OT::Scalar targetBeta_ = 0.0;
  OT::Bool fixedStep_ = true;
  OT::Scalar fixedStepValue_ = 1.0;
  OT::UnsignedInteger variableStepMaxIterations_ = 5;
  OT::UnsignedInteger maximumIteration_ = 100;
  OT::Scalar variableConvergence_ = 1e-3;
  OT::Scalar betaConvergence_ = 1e-2;
  OT::Scalar limitStateConvergence_ = 1e-3;
  OT::Bool stop_ = false;
};

}

#endif



