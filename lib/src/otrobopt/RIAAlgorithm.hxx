//                                               -*- C++ -*-
/**
 *  @brief Reliability Index Approach (RIA) for RBDO
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
#ifndef OTROBOPT_RIALGORITHM_HXX
#define OTROBOPT_RIALGORITHM_HXX

#include "otrobopt/RobustOptimizationAlgorithm.hxx"
#include "otrobopt/RBDOProblem.hxx"

#include <openturns/OptimizationAlgorithm.hxx>

namespace OTROBOPT
{

/**
 * @class RIAAlgorithm
 *
 * Reliability Index Approach (RIA) for RBDO.
 *
 * The outer optimization problem minimizes f(d, p) subject to:
 *   beta_k(d, p) - beta_target_k >= 0   for each probabilistic constraint k
 *   h_j(d, p) >= 0                       for each deterministic constraint j
 *
 * where beta_k is the Hasofer-Lind reliability index computed by FORM
 * for the k-th probabilistic constraint at the design point.
 */
class OTROBOPT_API RIAAlgorithm
  : public RobustOptimizationAlgorithm
{
  CLASSNAME

public:
  /** Default constructor */
  RIAAlgorithm();

  /** Parameter constructor with RBDOProblem */
  RIAAlgorithm(const RBDOProblem & rbdoProblem,
               const OT::OptimizationAlgorithm & solver);

  /** Virtual constructor */
  RIAAlgorithm * clone() const override;

  /** Starting point */
  void setStartingPoint(const OT::Point & startingPoint) override;
  OT::Point getStartingPoint() const override;

  /** Run the algorithm */
  void run() override;

  /** RBDO problem accessor */
  void setRBDOProblem(const RBDOProblem & rbdoProblem);
  RBDOProblem getRBDOProblem() const;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

private:
  RBDOProblem rbdoProblem_;
  OT::Point startingPoint_;

}; /* class RIAAlgorithm */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_RIALGORITHM_HXX */
