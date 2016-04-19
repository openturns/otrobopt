//                                               -*- C++ -*-
/**
 *  @brief Variance measure
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
#ifndef OTROBOPT_VARIANCEMEASURE_HXX
#define OTROBOPT_VARIANCEMEASURE_HXX

#include "otrobopt/MeasureEvaluationImplementation.hxx"

namespace OTROBOPT
{

/**
 * @class VarianceMeasure
 *
 * Variance measure
 */
class OTROBOPT_API VarianceMeasure
  : public MeasureEvaluationImplementation
{
  CLASSNAME;

public:
  /** Default constructor */
  VarianceMeasure();

  /** Parameter constructor */
  VarianceMeasure(const OT::Distribution & distribution,
                  const OT::NumericalMathFunction & function);

  /** Virtual constructor method */
  VarianceMeasure * clone() const;

  /** Evaluation */
  OT::NumericalPoint operator()(const OT::NumericalPoint & inP) const;

  /** String converter */
  OT::String __repr__() const;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv);

private:


}; /* class VarianceMeasure */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_VARIANCEMEASURE_HXX */
