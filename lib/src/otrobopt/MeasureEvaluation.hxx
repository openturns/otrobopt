//                                               -*- C++ -*-
/**
 *  @brief Measure function evaluation
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
#ifndef OTROBOPT_MEASUREEVALUATION_HXX
#define OTROBOPT_MEASUREEVALUATION_HXX

#include <openturns/TypedInterfaceObject.hxx>
#include <openturns/StorageManager.hxx>
#include <openturns/NumericalPoint.hxx>
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
  CLASSNAME;

public:

  /** Default constructor */
  MeasureEvaluation();

  MeasureEvaluation(const MeasureEvaluationImplementation & implementation);

  /** Distribution accessor */
  void setDistribution(const OT::Distribution & distribution);
  OT::Distribution getDistribution() const;

  /** Function accessor */
  void setFunction(const OT::NumericalMathFunction & function);
  OT::NumericalMathFunction getFunction() const;

  /** Evaluation */
  OT::NumericalPoint operator()(const OT::NumericalPoint & inP) const;

  /** String converter */
  OT::String __repr__() const;

private:

}; /* class MeasureEvaluation */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_MEASUREEVALUATION_HXX */
