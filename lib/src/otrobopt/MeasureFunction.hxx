//                                               -*- C++ -*-
/**
 *  @brief Measure function
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
#ifndef OTROBOPT_MEASUREFUNCTION_HXX
#define OTROBOPT_MEASUREFUNCTION_HXX

#include <openturns/NumericalMathFunctionImplementation.hxx>

#include "otrobopt/MeasureEvaluation.hxx"

namespace OTROBOPT
{

/**
 * @class MeasureFunction
 *
 * Measure function
 */
class OTROBOPT_API MeasureFunction
  : public OT::NumericalMathFunctionImplementation
{
  CLASSNAME;

public:

  /** Default constructor */
  MeasureFunction();

  explicit MeasureFunction(const MeasureEvaluation & evaluation);

  MeasureFunction * clone() const;

  /** Evaluation */
  virtual OT::NumericalPoint operator()(const OT::NumericalPoint & inP) const;
  virtual OT::NumericalSample operator()(const OT::NumericalSample & inS) const;
  virtual OT::UnsignedInteger getInputDimension() const;
  virtual OT::UnsignedInteger getOutputDimension() const;

  /** String converter */
  OT::String __repr__() const;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv);

private:
  MeasureEvaluation evaluation_;
}; /* class MeasureFunction */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_MEASUREFUNCTION_HXX */
