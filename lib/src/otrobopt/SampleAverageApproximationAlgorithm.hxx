//                                               -*- C++ -*-
/**
 *  @brief Sample-average approximation for reliability-based design optimization
 *
 *  Copyright 2005-2026 Airbus-EDF-IMACS-ONERA-Phimeca
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
#ifndef OTROBOPT_SAMPLEAVERAGEAPPROXIMATIONALGORITHM_HXX
#define OTROBOPT_SAMPLEAVERAGEAPPROXIMATIONALGORITHM_HXX

#include <openturns/OptimizationAlgorithm.hxx>
#include <openturns/OptimizationAlgorithmImplementation.hxx>

#include "otrobopt/OTRobOptprivate.hxx"
#include "otrobopt/RBDOProblem.hxx"

namespace OTROBOPT
{

/**
 * @class SampleAverageApproximationAlgorithm
 *
 * Solve a reliability-based design optimization problem by replacing the
 * chance constraint with its empirical counterpart on a fixed sample.
 *
 * The calibration constraint is the exact empirical safe probability, not a
 * smooth surrogate.  An independent validation sample and a Wilson confidence
 * interval are reported after the optimization.  The distribution parameter
 * is kept fixed.
 */
class OTROBOPT_API SampleAverageApproximationAlgorithm
  : public OT::OptimizationAlgorithmImplementation
{
  CLASSNAME

public:
  /** Default constructor */
  SampleAverageApproximationAlgorithm();

  /** Parameter constructor */
  SampleAverageApproximationAlgorithm(const RBDOProblem & problem,
                                      const OT::OptimizationAlgorithm & solver);

  /** Parameter constructor with a calibration sample size */
  SampleAverageApproximationAlgorithm(const RBDOProblem & problem,
                                      const OT::OptimizationAlgorithm & solver,
                                      const OT::UnsignedInteger sampleSize);

  /** Virtual constructor method */
  SampleAverageApproximationAlgorithm * clone() const override;

  /** Performs the actual computation */
  void run() override;

  /** Problem accessor */
  void setProblem(const OT::OptimizationProblem & problem) override;
  void setRBDOProblem(const RBDOProblem & problem);
  RBDOProblem getRBDOProblem() const;

  /** Outer optimization solver accessors */
  void setOptimizationAlgorithm(const OT::OptimizationAlgorithm & solver);
  OT::OptimizationAlgorithm getOptimizationAlgorithm() const;

  /** Calibration sample size accessors */
  void setSampleSize(const OT::UnsignedInteger sampleSize);
  OT::UnsignedInteger getSampleSize() const;

  /** Calibration sample accessors.  An empty sample restores random generation. */
  void setSample(const OT::Sample & sample);
  OT::Sample getSample() const;

  /** Validation sample size accessors */
  void setValidationSampleSize(const OT::UnsignedInteger sampleSize);
  OT::UnsignedInteger getValidationSampleSize() const;

  /** Validation sample accessors.  An empty sample restores random generation. */
  void setValidationSample(const OT::Sample & sample);
  OT::Sample getValidationSample() const;

  /** Empirical safe probability at the solution on the calibration sample */
  OT::Scalar getSafeProbability() const;

  /** Empirical safe probability at the solution on the validation sample */
  OT::Scalar getValidationSafeProbability() const;

  /** Wilson confidence interval of the validation safe probability */
  OT::Interval getValidationConfidenceInterval() const;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

protected:
  /** Check whether this problem can be solved by this solver */
  void checkProblem(const OT::OptimizationProblem & problem) const override;

private:
  OT::Point buildStartingPoint() const;
  OT::Sample buildCalibrationSample() const;
  OT::Sample buildValidationSample() const;

  RBDOProblem rbdoProblem_;
  OT::OptimizationAlgorithm solver_;
  OT::UnsignedInteger sampleSize_ = 1000;
  OT::UnsignedInteger validationSampleSize_ = 10000;
  OT::Sample sample_;
  OT::Sample validationSample_;
  OT::Bool sampleProvided_ = false;
  OT::Bool validationSampleProvided_ = false;

  OT::Scalar safeProbability_ = 0.0;
  OT::Scalar validationSafeProbability_ = 0.0;
  OT::Interval validationConfidenceInterval_;
  OT::Bool hasRun_ = false;

}; /* class SampleAverageApproximationAlgorithm */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_SAMPLEAVERAGEAPPROXIMATIONALGORITHM_HXX */
