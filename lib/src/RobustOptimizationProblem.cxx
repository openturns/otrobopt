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
#include "otrobopt/RobustOptimizationProblem.hxx"
#include <otrobopt/MeasureFunctionImplementation.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/UserDefined.hxx>
#include <openturns/LHSExperiment.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(RobustOptimizationProblem);

static Factory<RobustOptimizationProblem> RegisteredFactory;


/* Default constructor */
RobustOptimizationProblem::RobustOptimizationProblem()
  : OptimizationProblemImplementation()
{
  // Nothing to do
}

/* Parameter constructor */
RobustOptimizationProblem::RobustOptimizationProblem (const MeasureFunction & robustnessMeasure,
                                                      const MeasureFunction & reliabilityMeasure)
  : OptimizationProblemImplementation()
{
  setRobustnessMeasure(robustnessMeasure);
  setReliabilityMeasure(reliabilityMeasure);
}

/* Virtual constructor method */
RobustOptimizationProblem * RobustOptimizationProblem::clone() const
{
  return new RobustOptimizationProblem(*this);
}

/* String converter */
String RobustOptimizationProblem::__repr__() const
{
  OSS oss;
  oss << "class=" << RobustOptimizationProblem::GetClassName();
  return oss;
}

/* Measure accessors */
void RobustOptimizationProblem::setRobustnessMeasure(const MeasureFunction & robustnessMeasure)
{
  robustnessMeasure_ = robustnessMeasure;
  setObjective(robustnessMeasure.getFunction());
}

MeasureFunction RobustOptimizationProblem::getRobustnessMeasure() const
{
  return robustnessMeasure_;
}

Bool RobustOptimizationProblem::hasRobustnessMeasure() const
{
  return robustnessMeasure_.getFunction().getInputDimension() > 0;
}

void RobustOptimizationProblem::setReliabilityMeasure(const MeasureFunction & reliabilityMeasure)
{
  reliabilityMeasure_ = reliabilityMeasure;
  setInequalityConstraint(reliabilityMeasure.getFunction());
}

MeasureFunction RobustOptimizationProblem::getReliabilityMeasure() const
{
  return reliabilityMeasure_;
}

Bool RobustOptimizationProblem::hasReliabilityMeasure() const
{
  return reliabilityMeasure_.getFunction().getInputDimension() > 0;
}

/* Method save() stores the object through the StorageManager */
void RobustOptimizationProblem::save(Advocate & adv) const
{
  OptimizationProblemImplementation::save(adv);
  adv.saveAttribute("robustnessMeasure_", robustnessMeasure_);
  adv.saveAttribute("reliabilityMeasure_", reliabilityMeasure_);
}

/* Method load() reloads the object from the StorageManager */
void RobustOptimizationProblem::load(Advocate & adv)
{
  OptimizationProblemImplementation::load(adv);
  adv.loadAttribute("robustnessMeasure_", robustnessMeasure_);
  adv.loadAttribute("reliabilityMeasure_", reliabilityMeasure_);
}


} /* namespace OTROBOPT */
