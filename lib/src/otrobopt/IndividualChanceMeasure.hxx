//                                               -*- C++ -*-
/**
 *  @brief IndividualChanceMeasure
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
#ifndef OTROBOPT_INDIVIDUALCHANCEMEASURE_HXX
#define OTROBOPT_INDIVIDUALCHANCEMEASURE_HXX

#include "otrobopt/MeasureFunctionImplementation.hxx"

namespace OTROBOPT
{

/**
 * @class IndividualChanceMeasure
 *
 * Mean measure
 */
class OTROBOPT_API IndividualChanceMeasure
  : public MeasureFunctionImplementation
{
  CLASSNAME;

public:
  /** Default constructor */
  IndividualChanceMeasure();

  /** Parameter constructor */
  IndividualChanceMeasure (const OT::Distribution & distribution,
                      const OT::NumericalMathFunction & function,
                      const OT::NumericalScalar alpha);

  /** Virtual constructor method */
  IndividualChanceMeasure * clone() const;

  /** Evaluation */
  OT::NumericalPoint operator()(const OT::NumericalPoint & inP) const;

  /** Alpha coefficient accessor */
  void setAlpha(const OT::NumericalScalar alpha);
  OT::NumericalScalar getAlpha() const;

  /** Dimension accessor */
  virtual OT::UnsignedInteger getOutputDimension() const;

  /** String converter */
  OT::String __repr__() const;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv);

private:
  OT::NumericalScalar alpha_;

}; /* class IndividualChanceMeasure */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_INDIVIDUALCHANCEMEASURE_HXX */
