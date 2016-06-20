//                                               -*- C++ -*-
/**
 *  @brief Individual probability measure
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
#ifndef OTROBOPT_INDIVIDUALCHANCEMEASURE_HXX
#define OTROBOPT_INDIVIDUALCHANCEMEASURE_HXX

#include "otrobopt/MeasureEvaluationImplementation.hxx"

namespace OTROBOPT
{

/**
 * @class IndividualChanceMeasure
 *
 * Individual probability measure
 */
class OTROBOPT_API IndividualChanceMeasure
  : public MeasureEvaluationImplementation
{
  CLASSNAME;

public:
  /** Default constructor */
  IndividualChanceMeasure();

  /** Parameter constructor */
  IndividualChanceMeasure (const OT::NumericalMathFunction & function,
                           const OT::Distribution & distribution,
                           const OT::ComparisonOperator & op,
                           const OT::NumericalPoint & alpha);

  /** Virtual constructor method */
  IndividualChanceMeasure * clone() const;

  /** Evaluation */
  OT::NumericalPoint operator()(const OT::NumericalPoint & inP) const;

  /** Alpha coefficient accessor */
  void setAlpha(const OT::NumericalPoint & alpha);
  OT::NumericalPoint getAlpha() const;

  /** String converter */
  OT::String __repr__() const;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv);

private:
  OT::ComparisonOperator operator_;
  OT::NumericalPoint alpha_;

}; /* class IndividualChanceMeasure */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_INDIVIDUALCHANCEMEASURE_HXX */
