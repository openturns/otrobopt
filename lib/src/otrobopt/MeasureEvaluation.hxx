//                                               -*- C++ -*-
/**
 *  @brief Measure function evaluation
 *
 *  Copyright 2005-2024 Airbus-EDF-IMACS-Phimeca
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
#ifndef OTROBOPT_MEASUREEVALUATION_HXX
#define OTROBOPT_MEASUREEVALUATION_HXX

#include <openturns/TypedInterfaceObject.hxx>
#include <openturns/StorageManager.hxx>
#include <openturns/Distribution.hxx>

#include "otrobopt/OTRobOptprivate.hxx"

namespace OTROBOPT
{

/* forward declaration */
class MeasureEvaluationImplementation;

/**
 * @class MeasureEvaluation
 *
 * Measure function evaluation
 */
class OTROBOPT_API MeasureEvaluation
  : public OT::TypedInterfaceObject<MeasureEvaluationImplementation>
{
  CLASSNAME

public:

  /** Default constructor */
  MeasureEvaluation();

  MeasureEvaluation(const MeasureEvaluationImplementation & implementation);

  /** Distribution accessor */
  void setDistribution(const OT::Distribution & distribution);
  OT::Distribution getDistribution() const;

  /** Function accessor */
  void setFunction(const OT::Function & function);
  OT::Function getFunction() const;

  /** Evaluation */
  OT::Point operator()(const OT::Point & inP) const;

  /** String converter */
  OT::String __repr__() const override;

private:

}; /* class MeasureEvaluation */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_MEASUREEVALUATION_HXX */
