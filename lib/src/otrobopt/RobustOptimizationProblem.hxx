//                                               -*- C++ -*-
/**
 *  @brief Robust optimization problem
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
  CLASSNAME

public:
  /** Default constructor */
  RobustOptimizationProblem();

  RobustOptimizationProblem(const MeasureEvaluation & robustnessMeasure,
                            const MeasureEvaluation & reliabilityMeasure);

  RobustOptimizationProblem(const OT::Function & objective,
                            const MeasureEvaluation & reliabilityMeasure);

  RobustOptimizationProblem(const MeasureEvaluation & robustnessMeasure,
                            const OT::Function & constraint);

  /** Virtual constructor method */
  RobustOptimizationProblem * clone() const override;

  /** Measure accessors */
  void setRobustnessMeasure(const MeasureEvaluation & robustnessMeasure);
  MeasureEvaluation getRobustnessMeasure() const;
  OT::Bool hasRobustnessMeasure() const;

  void setReliabilityMeasure(const MeasureEvaluation & reliabilityMeasure);
  MeasureEvaluation getReliabilityMeasure() const;
  OT::Bool hasReliabilityMeasure() const;

  OT::Distribution getDistribution() const;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv) override;

private:
  MeasureEvaluation robustnessMeasure_;
  MeasureEvaluation reliabilityMeasure_;


}; /* class RobustOptimizationProblem */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_ROBUSTOPTIMIZATIONPROBLEM_HXX */
