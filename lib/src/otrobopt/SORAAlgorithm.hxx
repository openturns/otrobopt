//                                               -*- C++ -*-
/**
 *  @brief Sequential Optimization and Reliability Analysis (SORA) for RBDO
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
#ifndef OTROBOPT_SORAALGORITHM_HXX
#define OTROBOPT_SORAALGORITHM_HXX

#include "otrobopt/RobustOptimizationAlgorithm.hxx"
#include "otrobopt/RBDOProblem.hxx"

#include <openturns/OptimizationAlgorithm.hxx>
#include <openturns/OptimizationResult.hxx>
#include <openturns/Point.hxx>
#include <openturns/Sample.hxx>

namespace OTROBOPT
{

/**
 * @class SORAAlgorithm
 *
 * Sequential Optimization and Reliability Analysis (SORA) for RBDO.
 *
 * Iterates between:
 *   1. A deterministic optimization with the constraint evaluated at
 *      the MPTP from the previous iteration.
 *   2. Inverse FORM at the optimal design point to compute the new MPTP.
 *
 * Converges when the design point stabilizes between iterations.
 */
class OTROBOPT_API SORAAlgorithm
  : public RobustOptimizationAlgorithm
{
  CLASSNAME

public:
  typedef OT::Collection<OT::Point>           PointCollection;
  typedef OT::PersistentCollection<OT::Point> PointPersistentCollection;

  /** Default constructor */
  SORAAlgorithm();

  /** Parameter constructor (RBDOProblem) */
  SORAAlgorithm(const RBDOProblem & rbdoProblem,
                const OT::OptimizationAlgorithm & solver);

  /** Virtual constructor */
  SORAAlgorithm * clone() const override;

  /** Starting point */
  void setStartingPoint(const OT::Point & startingPoint) override;
  OT::Point getStartingPoint() const override;

  /** Run the algorithm */
  void run() override;

  /** RBDO problem accessor */
  void setRBDOProblem(const RBDOProblem & rbdoProblem);
  RBDOProblem getRBDOProblem() const;

  /** SORA-specific parameters */
  void setMaximumSORAIteration(OT::UnsignedInteger maxIter);
  OT::UnsignedInteger getMaximumSORAIteration() const;

  void setSORATolerance(OT::Scalar tolerance);
  OT::Scalar getSORATolerance() const;

  /** Most Probable Target Points accessor */
  PointCollection getMostProbableTargetPoints() const;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

private:
  RBDOProblem rbdoProblem_;
  OT::Point startingPoint_;
  OT::UnsignedInteger maxSORAIteration_;
  OT::Scalar soraTolerance_;
  PointPersistentCollection mostProbableTargetPoints_;

}; /* class SORAAlgorithm */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_SORAALGORITHM_HXX */
