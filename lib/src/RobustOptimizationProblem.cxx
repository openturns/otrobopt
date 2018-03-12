//                                               -*- C++ -*-
/**
 *  @brief Robust optimization problem
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
#include "otrobopt/RobustOptimizationProblem.hxx"
#include <otrobopt/MeasureEvaluationImplementation.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/FunctionImplementation.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(RobustOptimizationProblem)

static Factory<RobustOptimizationProblem> Factory_RobustOptimizationProblem;


/* Default constructor */
RobustOptimizationProblem::RobustOptimizationProblem()
  : OptimizationProblemImplementation()
{
  // Nothing to do
}

RobustOptimizationProblem::RobustOptimizationProblem(const MeasureEvaluation & robustnessMeasure,
                                                     const MeasureEvaluation & reliabilityMeasure)
  : OptimizationProblemImplementation()
{
  setRobustnessMeasure(robustnessMeasure);
  setReliabilityMeasure(reliabilityMeasure);
  if (robustnessMeasure.getDistribution() != reliabilityMeasure.getDistribution())
    throw InvalidArgumentException(HERE) << "Different measure distributions for objective and constraint";
}

RobustOptimizationProblem::RobustOptimizationProblem(const Function & objective,
                                                     const MeasureEvaluation & reliabilityMeasure)
  : OptimizationProblemImplementation()
{
  setObjective(objective);
  setReliabilityMeasure(reliabilityMeasure);
}

RobustOptimizationProblem::RobustOptimizationProblem(const MeasureEvaluation & robustnessMeasure,
                                                     const Function & inequalityConstraint)
  : OptimizationProblemImplementation()
{
  setRobustnessMeasure(robustnessMeasure);
  setInequalityConstraint(inequalityConstraint);
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
  oss << "class=" << RobustOptimizationProblem::GetClassName()
      << " robustnessMeasure=" << robustnessMeasure_
      << " reliabilityMeasure=" << reliabilityMeasure_;
  return oss;
}

/* Measure accessors */
void RobustOptimizationProblem::setRobustnessMeasure(const MeasureEvaluation & robustnessMeasure)
{
  robustnessMeasure_ = robustnessMeasure;
  setObjective(FunctionImplementation(robustnessMeasure.getImplementation()));
}

MeasureEvaluation RobustOptimizationProblem::getRobustnessMeasure() const
{
  if (!hasRobustnessMeasure()) throw InvalidArgumentException(HERE) << "No robustness measure defined";
  return robustnessMeasure_;
}

Bool RobustOptimizationProblem::hasRobustnessMeasure() const
{
  return robustnessMeasure_.getFunction().getInputDimension() > 0;
}

void RobustOptimizationProblem::setReliabilityMeasure(const MeasureEvaluation & reliabilityMeasure)
{
  reliabilityMeasure_ = reliabilityMeasure;
  setInequalityConstraint(FunctionImplementation(reliabilityMeasure.getImplementation()));
}

MeasureEvaluation RobustOptimizationProblem::getReliabilityMeasure() const
{
  if (!hasReliabilityMeasure()) throw InvalidArgumentException(HERE) << "No reliability measure defined";
  return reliabilityMeasure_;
}

Bool RobustOptimizationProblem::hasReliabilityMeasure() const
{
  return reliabilityMeasure_.getFunction().getInputDimension() > 0;
}

Distribution RobustOptimizationProblem::getDistribution() const
{
  Distribution distributionXi;
  if (hasRobustnessMeasure())
    distributionXi = getRobustnessMeasure().getDistribution();
  if (hasReliabilityMeasure())
    distributionXi = getReliabilityMeasure().getDistribution();
  return distributionXi;
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
