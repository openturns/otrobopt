//                                               -*- C++ -*-
/**
 *  @brief Alternating discretization of measures and deterministic optimization steps
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
#ifndef OTROBOPT_SEQUENTIALMONTECARLOROBUSTALGORITHM_HXX
#define OTROBOPT_SEQUENTIALMONTECARLOROBUSTALGORITHM_HXX

#include "otrobopt/RobustOptimizationAlgorithm.hxx"

namespace OTROBOPT
{

/**
 * @class SequentialMonteCarloRobustAlgorithm
 *
 * Alternating discretization of measures and deterministic optimization steps
 */
class OTROBOPT_API SequentialMonteCarloRobustAlgorithm
  : public RobustOptimizationAlgorithm
{
  CLASSNAME;

public:
  /** Default constructor */
  SequentialMonteCarloRobustAlgorithm();

  /** Parameter constructor */
  SequentialMonteCarloRobustAlgorithm(const RobustOptimizationProblem & problem,
                                      const OT::OptimizationSolver & solver);

  /** Virtual constructor method */
  SequentialMonteCarloRobustAlgorithm * clone() const;

  /** Evaluation */
  virtual void run();

  /** Initial sampling size accessor */
  void setInitialSamplingSize(const OT::UnsignedInteger N0);
  OT::UnsignedInteger getInitialSamplingSize() const;

  void setInitialSearch(const OT::UnsignedInteger initialSearch);
  OT::UnsignedInteger getInitialSearch() const;

  /** String converter */
  OT::String __repr__() const;

  /** Method save() stores the object through the StorageManager */
  virtual void save(OT::Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  virtual void load(OT::Advocate & adv);

private:
  // initial sampling size
  OT::UnsignedInteger N0_;

  // number of sampled initial points
  OT::UnsignedInteger initialSearch_;

}; /* class SequentialMonteCarloRobustAlgorithm */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_SEQUENTIALMONTECARLOROBUSTALGORITHM_HXX */
