//                                               -*- C++ -*-
/**
 *  @brief Tradeoff between mean and standard deviation
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
#ifndef OTROBOPT_MEANSTANDARDDEVIATIONTRADEOFFMEASURE_HXX
#define OTROBOPT_MEANSTANDARDDEVIATIONTRADEOFFMEASURE_HXX

#include "otrobopt/MeasureEvaluationImplementation.hxx"

namespace OTROBOPT
{

/**
 * @class MeanStandardDeviationTradeoffMeasure
 *
 * Tradeoff between mean and standard deviation
 */
class OTROBOPT_API MeanStandardDeviationTradeoffMeasure
  : public MeasureEvaluationImplementation
{
  CLASSNAME;

public:
  /** Default constructor */
  MeanStandardDeviationTradeoffMeasure();

  /** Parameter constructor */
  MeanStandardDeviationTradeoffMeasure (const OT::Function & function,
                                        const OT::Distribution & distribution,
                                        const OT::Point & alpha);

  /** Virtual constructor method */
  MeanStandardDeviationTradeoffMeasure * clone() const;

  /** Evaluation */
  OT::Point operator()(const OT::Point & inP) const;

  /** Alpha coefficient accessor */
  void setAlpha(const OT::Point & alpha);
  OT::Point getAlpha() const;

  /** String converter */
  OT::String __repr__() const;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv);

private:
  OT::Point alpha_;

}; /* class MeanStandardDeviationTradeoffMeasure */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_MEANSTANDARDDEVIATIONTRADEOFFMEASURE_HXX */
