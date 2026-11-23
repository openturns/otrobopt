//                                               -*- C++ -*-
/**
 *  @brief Reliability-based estimator of a system-event probability
 *
 *  Copyright 2005-2026 Airbus-EDF-IMACS-ONERA-Phimeca
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
#ifndef OTROBOPT_RELIABILITYESTIMATOR_HXX
#define OTROBOPT_RELIABILITYESTIMATOR_HXX

#include "otrobopt/ProbabilityEstimatorImplementation.hxx"
#include <openturns/OptimizationAlgorithm.hxx>

namespace OTROBOPT
{

/**
 * @class ReliabilityEstimator
 *
 * First-order estimator of the probability of a Boolean system event.
 *
 * Each of the m limit-state components is analysed by FORM, optionally
 * restarted to collect several design points (MultiFORM).  The component
 * failure event of leaf j is linearized as
 * {c_j(u) = a_j^T u >= beta_j}, and the safe event as {a_j^T u <= beta_j};
 * the signed reliability index beta_j carries the orientation, so that a safe
 * event with a probability larger than one half is represented without
 * clamping.  The system event is then the Boolean predicate applied to the m
 * linearized component indicators, and its probability is combined exactly
 * over the 2^N atoms of the N linearized half-spaces, where atom probabilities
 * are orthant probabilities of the standard normal law with correlation
 * matrix R_ij = a_i.a_j.  Exactly parallel and anti-parallel half-spaces are
 * reduced to intervals of their common standard normal projection, so that a
 * MultiFORM component with symmetric mechanisms does not build a singular
 * multinormal law.
 *
 * The exact combination evaluates one multinormal distribution function (or a
 * signed sum of them for the interval groups) per atom.  It is attempted only
 * when 2^N does not exceed
 * getMaximumCombinationNumber(); above that bound the estimator returns the
 * midpoint of the first-order bounds obtained from the component marginals and
 * reports their half-width as its estimation error.  The probability is then a
 * bounded estimate, never a silently truncated exact sum.
 *
 * The analytical combination requires a normal standard space, that is a
 * distribution whose standard distribution is the standard normal law, because
 * the multinormal orthant probability is valid only there.  Otherwise the
 * system algorithm must be SIMULATION, which samples the distribution, applies
 * the predicate to the component indicators and reports the binomial standard
 * error.
 */
class OTROBOPT_API ReliabilityEstimator
  : public ProbabilityEstimatorImplementation
{
  CLASSNAME

public:
  /** Default constructor */
  ReliabilityEstimator();

  /** Parameter constructor */
  ReliabilityEstimator(const OT::String & componentAlgorithm,
                       const OT::String & systemAlgorithm);

  /** Virtual constructor */
  ReliabilityEstimator * clone() const override;

  /** Whether the estimator provides a geometry */
  OT::Bool hasGeometry() const override;

  /** Signed reliability index of each limit-state component */
  OT::Point getComponentReliabilityIndexes(const OT::Function & predicate,
                                           const OT::Function & limitState,
                                           const OT::Distribution & distribution,
                                           const OT::Point & designPoint) const override;

  /** Standard-space design points, grouped by limit-state component */
  OT::Sample getStandardDesignPoints(const OT::Function & predicate,
                                     const OT::Function & limitState,
                                     const OT::Distribution & distribution,
                                     const OT::Point & designPoint) const override;

  /** Number of standard-space design points of each limit-state component */
  OT::Indices getDesignPointsNumber(const OT::Function & predicate,
                                    const OT::Function & limitState,
                                    const OT::Distribution & distribution,
                                    const OT::Point & designPoint) const override;

  /** Bounds of the probability of the linearized event */
  OT::Interval getProbabilityBounds(const OT::Function & predicate,
                                    const OT::Function & limitState,
                                    const OT::Distribution & distribution,
                                    const OT::Point & designPoint) const override;

  /** Component algorithm accessor, FORM or MULTIFORM */
  void setComponentAlgorithm(const OT::String & algorithm);
  OT::String getComponentAlgorithm() const;

  /** System algorithm accessor, ANALYTICAL or SIMULATION */
  void setSystemAlgorithm(const OT::String & algorithm);
  OT::String getSystemAlgorithm() const;

  /** Nearest-point solver accessor */
  void setSolver(const OT::OptimizationAlgorithm & solver);
  OT::OptimizationAlgorithm getSolver() const;

  /** Maximum number of design points of one component */
  void setMaximumDesignPointsNumber(const OT::UnsignedInteger number);
  OT::UnsignedInteger getMaximumDesignPointsNumber() const;

  /** Maximum number of atoms of the exact combination */
  void setMaximumCombinationNumber(const OT::UnsignedInteger number);
  OT::UnsignedInteger getMaximumCombinationNumber() const;

  /** Nominal reliability index used to seed the inner analyses */
  void setBetaTarget(const OT::Scalar betaTarget);
  OT::Scalar getBetaTarget() const;

  /** Sample size of the simulation system algorithm */
  void setSampleSize(const OT::UnsignedInteger size);
  OT::UnsignedInteger getSampleSize() const;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

protected:
  /** Probability implementation */
  OT::Scalar computeSafeProbability(const OT::Function & predicate,
                                    const OT::Function & limitState,
                                    const OT::Distribution & distribution,
                                    const OT::Point & designPoint) const override;

  /** Error implementation */
  OT::Scalar computeEstimationError(const OT::Function & predicate,
                                    const OT::Function & limitState,
                                    const OT::Distribution & distribution,
                                    const OT::Point & designPoint) const override;

private:
  /** Result of one reliability analysis */
  struct Analysis
  {
    OT::Scalar safeProbability = 0.0;
    OT::Scalar estimationError = 0.0;
    OT::Scalar safeProbabilityLowerBound = 0.0;
    OT::Scalar safeProbabilityUpperBound = 1.0;
    OT::Bool exact = true;
    OT::Point componentReliabilityIndexes;
    OT::Sample standardDesignPoints;
    OT::Indices designPointsNumber;
  };

  /** Run the analysis at the design point */
  Analysis analyze(const OT::Function & predicate,
                   const OT::Function & limitState,
                   const OT::Distribution & distribution,
                   const OT::Point & designPoint) const;

  /** Per-component design points: standard point and signed reliability index */
  struct ComponentDesignPoint
  {
    OT::Point standardPoint;
    OT::Scalar reliabilityIndex = 0.0;
  };
  typedef OT::Collection<ComponentDesignPoint> ComponentDesignPointCollection;

  /** Design points of one limit-state component */
  ComponentDesignPointCollection solveComponent(const OT::Function & standardFunction) const;

  /** Analytical system probability over the linearized components */
  void combineSystem(const OT::Function & predicate,
                     const OT::UnsignedInteger componentNumber,
                     const OT::Collection<ComponentDesignPointCollection> & componentDesignPoints,
                     Analysis & analysis) const;

  /** Simulation system probability */
  void simulateSystem(const OT::Function & predicate,
                      const OT::Function & limitState,
                      const OT::Distribution & distribution,
                      const OT::Point & designPoint,
                      Analysis & analysis) const;

  OT::String componentAlgorithm_ = "FORM";
  OT::String systemAlgorithm_ = "ANALYTICAL";
  OT::OptimizationAlgorithm solver_;
  OT::UnsignedInteger maximumDesignPointsNumber_ = 1;
  OT::UnsignedInteger maximumCombinationNumber_ = 1024;
  OT::Scalar betaTarget_ = 2.0;
  OT::UnsignedInteger sampleSize_ = 10000;

}; /* class ReliabilityEstimator */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_RELIABILITYESTIMATOR_HXX */
