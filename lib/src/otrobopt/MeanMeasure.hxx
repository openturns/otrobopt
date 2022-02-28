//                                               -*- C++ -*-
/**
 *  @brief Mean measure
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
#ifndef OTROBOPT_MEANMEASURE_HXX
#define OTROBOPT_MEANMEASURE_HXX

#include "otrobopt/MeasureEvaluationImplementation.hxx"

namespace OTROBOPT
{

/**
 * @class MeanMeasure
 *
 * Mean measure
 */
class OTROBOPT_API MeanMeasure
  : public MeasureEvaluationImplementation
{
  CLASSNAME

public:
  /** Default constructor */
  MeanMeasure();

  /** Parameter constructor */
  MeanMeasure(const OT::Function & function,
              const OT::Distribution & distribution);

  /** Virtual constructor method */
  MeanMeasure * clone() const override;

  /** Evaluation */
  OT::Point operator()(const OT::Point & inP) const override;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

private:

}; /* class MeanMeasure */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_MEANMEASURE_HXX */
