//                                               -*- C++ -*-
/**
 *  @brief Measure function evaluation implementation
 *
 *  Copyright 2005-2018 Airbus-EDF-IMACS-Phimeca
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
#ifndef OTROBOPT_MEASUREEVALUATIONIMPLEMENTATION_HXX
#define OTROBOPT_MEASUREEVALUATIONIMPLEMENTATION_HXX

#include "otrobopt/OTRobOptprivate.hxx"

#include <openturns/Distribution.hxx>

namespace OTROBOPT
{

/**
 * @class MeasureEvaluationImplementation
 *
 * Measure function evaluation implementation
 */
class OTROBOPT_API MeasureEvaluationImplementation
  : public OT::EvaluationImplementation
{
  CLASSNAME

public:
  /** Default constructor */
  MeasureEvaluationImplementation();

  /** Parameter constructor */
  MeasureEvaluationImplementation(const OT::Function & function,
                                  const OT::Distribution & distribution);

  /** Virtual constructor method */
  MeasureEvaluationImplementation * clone() const;

  /** Evaluation */
  virtual OT::Point operator()(const OT::Point & inP) const;

  /** Distribution accessor */
  virtual void setDistribution(const OT::Distribution & distribution);
  virtual OT::Distribution getDistribution() const;

  /** Function accessor */
  virtual void setFunction(const OT::Function & function);
  virtual OT::Function getFunction() const;

  /** Dimension accessor */
  virtual OT::UnsignedInteger getInputDimension() const;
  virtual OT::UnsignedInteger getOutputDimension() const;

  /** String converter */
  virtual OT::String __repr__() const;
  virtual OT::String __str__(const OT::String & offset = "") const;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv);

private:
  OT::Function function_;
  OT::Distribution distribution_;

}; /* class MeasureEvaluationImplementation */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_MEASUREEVALUATIONIMPLEMENTATION_HXX */
