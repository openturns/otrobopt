//                                               -*- C++ -*-
/**
 *  @brief Function whose parameter is exposed as input coordinates
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
#ifndef OTROBOPT_PARAMETERASINPUTFUNCTION_HXX
#define OTROBOPT_PARAMETERASINPUTFUNCTION_HXX

#include <openturns/Description.hxx>
#include <openturns/Function.hxx>
#include <openturns/FunctionImplementation.hxx>

#include "otrobopt/OTRobOptprivate.hxx"

namespace OTROBOPT
{

/**
 * @class ParameterAsInputFunction
 *
 * Function whose parameter is exposed as input coordinates.
 *
 * If f has input dimension d, output dimension p and parameter dimension q,
 * then H = ParameterAsInputFunction(f) has input dimension d + q, output
 * dimension p and no parameter, and is defined by:
 *
 *   H(u, theta) = f_theta(u)
 *
 * where f_theta is the original function evaluated with parameter theta.
 */
class OTROBOPT_API ParameterAsInputFunction
  : public OT::FunctionImplementation
{
  CLASSNAME

public:

  /** Default constructor */
  ParameterAsInputFunction();

  /** Parameter constructor */
  explicit ParameterAsInputFunction(const OT::Function & function);

  /** Virtual constructor */
  ParameterAsInputFunction * clone() const override;

  /** Evaluation */
  OT::Point operator()(const OT::Point & inP) const override;
  OT::Sample operator()(const OT::Sample & inS) const override;

  /** Gradient against the input coordinates */
  OT::Matrix gradient(const OT::Point & inP) const override;

  /** Gradient against the parameter: empty, the parameter is now an input */
  OT::Matrix parameterGradient(const OT::Point & inP) const override;
  void setParameter(const OT::Point & parameter) override;

  /** Dimension accessors */
  OT::UnsignedInteger getInputDimension() const override;
  OT::UnsignedInteger getOutputDimension() const override;

  /** Description accessors */
  OT::Description getInputDescription() const override;
  OT::Description getOutputDescription() const override;

  /** Function accessor */
  OT::Function getFunction() const;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

private:
  void initializeDescriptions();

  OT::Function function_;
  OT::Description inputDescription_;
  OT::Description outputDescription_;
}; /* class ParameterAsInputFunction */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_PARAMETERASINPUTFUNCTION_HXX */
