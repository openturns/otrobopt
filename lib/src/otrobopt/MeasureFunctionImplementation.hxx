//                                               -*- C++ -*-
/**
 *  @brief MeasureFunctionImplementation
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
#ifndef OTROBOPT_MEASUREFUNCTIONIMPLEMENTATION_HXX
#define OTROBOPT_MEASUREFUNCTIONIMPLEMENTATION_HXX

#include "otrobopt/OTRobOptprivate.hxx"

#include <openturns/Distribution.hxx>

namespace OTROBOPT
{

/**
 * @class MeasureFunctionImplementation
 *
 * MeasureFunctionImplementation is some measurefunction type to illustrate how to add some classes in OpenTURNS
 */
class OTROBOPT_API MeasureFunctionImplementation
  : public OT::NumericalMathFunctionImplementation
{
  CLASSNAME;

public:
  /** Default constructor */
  MeasureFunctionImplementation();

  /** Parameter constructor */
  MeasureFunctionImplementation(const OT::Distribution & distribution,
                                const OT::NumericalMathFunction & function);

  /** Virtual constructor method */
  MeasureFunctionImplementation * clone() const;

  /** Evaluation */
  virtual OT::NumericalPoint operator()(const OT::NumericalPoint & inP) const;

  /** Method gradient() returns the Jacobian transposed matrix of the function at point */
  virtual OT::Matrix gradient(const OT::NumericalPoint & inP) const;

  /** Distribution accessor */
  virtual void setDistribution(const OT::Distribution & distribution);
  virtual OT::Distribution getDistribution() const;

  /** Function accessor */
  virtual void setFunction(const OT::NumericalMathFunction & function);
  virtual OT::NumericalMathFunction getFunction() const;

  /** Dimension accessor */
  virtual OT::UnsignedInteger getInputDimension() const;
  virtual OT::UnsignedInteger getOutputDimension() const;

  /** String converter */
  virtual OT::String __repr__() const;
  virtual OT::String __str__(const OT::String & offset = "") const;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv);

private:
  OT::Distribution distribution_;
  OT::NumericalMathFunction function_;

}; /* class MeasureFunctionImplementation */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_MEASUREFUNCTIONIMPLEMENTATION_HXX */
