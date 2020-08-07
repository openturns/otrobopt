//                                               -*- C++ -*-
/**
 *  @brief Worst-case measure evaluation
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
#ifndef OTROBOPT_WORSTCASEMEASURE_HXX
#define OTROBOPT_WORSTCASEMEASURE_HXX

#include <openturns/OptimizationAlgorithm.hxx>
#include "otrobopt/MeasureEvaluationImplementation.hxx"

namespace OTROBOPT
{

/**
 * @class WorstCaseMeasure
 *
 * Worst-case measure evaluation
 */
class OTROBOPT_API WorstCaseMeasure
  : public MeasureEvaluationImplementation
{
  CLASSNAME

public:
  /** Default constructor */
  WorstCaseMeasure();

  /** Parameter constructor */
  WorstCaseMeasure (const OT::Function & function,
                    const OT::Distribution & distribution,
                    const OT::Bool minimization = true);

  /** Virtual constructor method */
  WorstCaseMeasure * clone() const override;

  /** Evaluation */
  OT::Point operator()(const OT::Point & inP) const override;

  /** Optimization solver accessor */
  void setOptimizationAlgorithm(const OT::OptimizationAlgorithm & solver);
  OT::OptimizationAlgorithm getOptimizationAlgorithm() const;

  /** Minimization accessor */
  void setMinimization(OT::Bool minimization);
  OT::Bool isMinimization() const;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv) override;

private:
  OT::Bool isMinimization_;
  OT::OptimizationAlgorithm solver_;

}; /* class WorstCaseMeasure */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_WORSTCASEMEASURE_HXX */
