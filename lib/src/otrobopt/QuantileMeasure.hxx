//                                               -*- C++ -*-
/**
 *  @brief Measure that evaluates as a quantile
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
#ifndef OTROBOPT_QUANTILEMEASURE_HXX
#define OTROBOPT_QUANTILEMEASURE_HXX

#include "otrobopt/MeasureEvaluationImplementation.hxx"

namespace OTROBOPT
{

/**
 * @class QuantileMeasure
 *
 * Measure that evaluates as a quantile
 */
class OTROBOPT_API QuantileMeasure
  : public MeasureEvaluationImplementation
{
  CLASSNAME;

public:
  /** Default constructor */
  QuantileMeasure();

  /** Parameter constructor */
  QuantileMeasure(const OT::Function & function,
                  const OT::Distribution & distribution,
                  const OT::Scalar alpha);

  /** Virtual constructor method */
  QuantileMeasure * clone() const;

  /** Evaluation */
  OT::Point operator()(const OT::Point & inP) const;

  /** Alpha coefficient accessor */
  void setAlpha(const OT::Scalar alpha);
  OT::Scalar getAlpha() const;

  /** String converter */
  OT::String __repr__() const;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv);

private:
  OT::Scalar alpha_;

}; /* class QuantileMeasure */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_QUANTILEMEASURE_HXX */
