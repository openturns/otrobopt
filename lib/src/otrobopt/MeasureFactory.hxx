//                                               -*- C++ -*-
/**
 *  @brief Measure discretization
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
#ifndef OTROBOPT_MEASUREFACTORY_HXX
#define OTROBOPT_MEASUREFACTORY_HXX

#include <openturns/Distribution.hxx>
#include <openturns/WeightedExperiment.hxx>

#include "otrobopt/MeasureEvaluation.hxx"

namespace OTROBOPT
{

/**
 * @class MeasureFactory
 *
 * Measure discretization
 */
class OTROBOPT_API MeasureFactory
  : public OT::PersistentObject
{
  CLASSNAME;


public:
  typedef OT::Collection<MeasureEvaluation> MeasureEvaluationCollection;

  /** Default constructor */
  MeasureFactory();

  /** Parameter constructor */
  MeasureFactory (const OT::WeightedExperiment & experiment);

  /** Virtual constructor method */
  MeasureFactory * clone() const;

  /** Evaluation */
  MeasureEvaluation build(const MeasureEvaluation & measure) const;
  MeasureEvaluationCollection buildCollection(const MeasureEvaluationCollection & collection) const;

  /** String converter */
  OT::String __repr__() const;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv);

private:
  OT::Pointer<OT::WeightedExperiment> p_experiment_;
}; /* class MeasureFactory */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_MEASUREFACTORY_HXX */
