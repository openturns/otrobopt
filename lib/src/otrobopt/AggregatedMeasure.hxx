//                                               -*- C++ -*-
/**
 *  @brief Aggregation of measure functions
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
#ifndef OTROBOPT_AGGREGATEDMEASURE_HXX
#define OTROBOPT_AGGREGATEDMEASURE_HXX

#include "otrobopt/MeasureEvaluationImplementation.hxx"
#include "otrobopt/MeasureEvaluation.hxx"

namespace OTROBOPT
{

/**
 * @class AggregatedMeasure
 *
 * Aggregation of measure functions
 */
class OTROBOPT_API AggregatedMeasure
  : public MeasureEvaluationImplementation
{
  CLASSNAME;

  typedef OT::Collection<MeasureEvaluation>           MeasureEvaluationCollection;
  typedef OT::PersistentCollection<MeasureEvaluation> MeasureEvaluationPersistentCollection;

public:
  /** Default constructor */
  AggregatedMeasure();

  /** Parameter constructor */
  AggregatedMeasure(const MeasureEvaluationCollection & collection);

  /** Virtual constructor method */
  AggregatedMeasure * clone() const;

  /** Evaluation */
  OT::NumericalPoint operator()(const OT::NumericalPoint & inP) const;

  /** Distribution accessor */
  virtual void setDistribution(const OT::Distribution & distribution);
  virtual OT::Distribution getDistribution() const;
  
  /** String converter */
  OT::String __repr__() const;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv);

private:
  MeasureEvaluationPersistentCollection collection_;

}; /* class AggregatedMeasure */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_AGGREGATEDMEASURE_HXX */
