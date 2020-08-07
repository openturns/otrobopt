//                                               -*- C++ -*-
/**
 *  @brief Measure function
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
#ifndef OTROBOPT_MEASUREFUNCTION_HXX
#define OTROBOPT_MEASUREFUNCTION_HXX

#include <openturns/FunctionImplementation.hxx>

#include "otrobopt/MeasureEvaluation.hxx"

namespace OTROBOPT
{

/**
 * @class MeasureFunction
 *
 * Measure function
 */
class OTROBOPT_API MeasureFunction
  : public OT::FunctionImplementation
{
  CLASSNAME

public:

  /** Default constructor */
  MeasureFunction();

  explicit MeasureFunction(const MeasureEvaluation & evaluation);

  MeasureFunction * clone() const override;

  /** Evaluation */
  OT::Point operator()(const OT::Point & inP) const override;
  OT::Sample operator()(const OT::Sample & inS) const override;
  OT::UnsignedInteger getInputDimension() const override;
  OT::UnsignedInteger getOutputDimension() const override;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

private:
  MeasureEvaluation evaluation_;
}; /* class MeasureFunction */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_MEASUREFUNCTION_HXX */
