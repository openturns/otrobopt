//                                               -*- C++ -*-
/**
 *  @brief Robust optimization problem
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
#ifndef OTROBOPT_ROBUSTOPTIMIZATIONPROBLEM_HXX
#define OTROBOPT_ROBUSTOPTIMIZATIONPROBLEM_HXX

#include <openturns/OptimizationProblemImplementation.hxx>

#include "otrobopt/MeasureEvaluation.hxx"
#include "otrobopt/MeasureEvaluationImplementation.hxx"

namespace OTROBOPT
{

/**
 * @class RobustOptimizationProblem
 *
 * Robust optimization problem
 */
class OTROBOPT_API RobustOptimizationProblem
  : public OT::OptimizationProblemImplementation
{
  CLASSNAME;

public:
  /** Default constructor */
  RobustOptimizationProblem();

  /** Parameter constructor */
  RobustOptimizationProblem(const MeasureEvaluation & robustnessMeasure,
                            const MeasureEvaluation & reliabilityMeasure);

  /** Virtual constructor method */
  RobustOptimizationProblem * clone() const;

  /** Measure accessors */
  void setRobustnessMeasure(const MeasureEvaluation & robustnessMeasure);
  MeasureEvaluation getRobustnessMeasure() const;
  OT::Bool hasRobustnessMeasure() const;

  void setReliabilityMeasure(const MeasureEvaluation & reliabilityMeasure);
  MeasureEvaluation getReliabilityMeasure() const;
  OT::Bool hasReliabilityMeasure() const;

  /** String converter */
  OT::String __repr__() const;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv);

private:
  MeasureEvaluation robustnessMeasure_;
  MeasureEvaluation reliabilityMeasure_;


}; /* class RobustOptimizationProblem */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_ROBUSTOPTIMIZATIONPROBLEM_HXX */
