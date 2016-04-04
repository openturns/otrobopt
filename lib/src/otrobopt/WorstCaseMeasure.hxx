//                                               -*- C++ -*-
/**
 *  @brief WorstCaseMeasure
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
#ifndef OTROBOPT_WORSTCASEMEASURE_HXX
#define OTROBOPT_WORSTCASEMEASURE_HXX

#include <openturns/OptimizationSolver.hxx>
#include "otrobopt/MeasureFunctionImplementation.hxx"

namespace OTROBOPT
{

/**
 * @class WorstCaseMeasure
 *
 * Mean measure
 */
class OTROBOPT_API WorstCaseMeasure
  : public MeasureFunctionImplementation
{
  CLASSNAME;

public:
  /** Default constructor */
  WorstCaseMeasure();

  /** Parameter constructor */
  WorstCaseMeasure (const OT::Distribution & distribution,
                    const OT::NumericalMathFunction & function,
                    const OT::Bool minimization = true);

  /** Virtual constructor method */
  WorstCaseMeasure * clone() const;

  /** Evaluation */
  OT::NumericalPoint operator()(const OT::NumericalPoint & inP) const;

  /** Optimization solver accessor */
  void setOptimizationSolver(const OT::OptimizationSolver & solver);
  OT::OptimizationSolver getOptimizationSolver() const;

  /** Minimization accessor */
  void setMinimization(OT::Bool minimization);
  OT::Bool isMinimization() const;

  /** String converter */
  OT::String __repr__() const;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv);

private:
  OT::Bool isMinimization_;
  OT::OptimizationSolver solver_;

}; /* class WorstCaseMeasure */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_WORSTCASEMEASURE_HXX */
