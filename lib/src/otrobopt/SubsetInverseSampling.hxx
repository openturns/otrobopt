//                                               -*- C++ -*-
/**
 *  @brief SubsetInverseSampling
 *
 *  Copyright 2005-2016 Airbus-EDF-IMACS-Phimeca
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
#ifndef OTROBOPT_SUBSETINVERSESAMPLING_HXX
#define OTROBOPT_SUBSETINVERSESAMPLING_HXX

#include <openturns/EventSimulation.hxx>
#include <openturns/StandardEvent.hxx>
#include "otrobopt/SubsetInverseSamplingResult.hxx"

namespace OTROBOPT
{


class OTROBOPT_API SubsetInverseSampling
: public OT::EventSimulation
{
CLASSNAME
public:

  typedef OT::Collection<OT::Sample> SampleCollection;
  typedef OT::PersistentCollection<OT::Sample> SamplePersistentCollection;

  /** Default Parameters */
  static const OT::UnsignedInteger DefaultMaximumOuterSampling;
  static const OT::Scalar DefaultProposalRange;
  static const OT::Scalar DefaultConditionalProbability;
  static const OT::Scalar DefaultBetaMin;

  /** Default Constructor */
  SubsetInverseSampling();

  /** Constructor with parameters */
  SubsetInverseSampling(const OT::RandomVector & event,
                 const OT::Scalar targetProbability,
                 const OT::Scalar proposalRange = DefaultProposalRange,
                 const OT::Scalar conditionalProbability = DefaultConditionalProbability);

  /** Virtual constructor */
  SubsetInverseSampling * clone() const override;

  /** Result accessor */
  SubsetInverseSamplingResult getResult() const;

  /** The range of the uniform proposal pdf */
  void setProposalRange(OT::Scalar proposalRange);
  OT::Scalar getProposalRange() const;

  /** Ratio parameter */
  void setConditionalProbability(OT::Scalar conditionalProbability);
  OT::Scalar getConditionalProbability() const;

  /** final target probability */
  void setTargetProbability(OT::Scalar targetProbability);
  OT::Scalar getTargetProbability() const;

  /** Accessor to the achieved number of steps */
  OT::UnsignedInteger getNumberOfSteps();

  OT::Scalar getThresholdConfidenceLength(const OT::Scalar level = OT::ResourceMap::GetAsScalar("ProbabilitySimulationResult-DefaultConfidenceLevel")) const;

  /** Stepwise result accessors */
  OT::Point getThresholdPerStep() const;
  OT::Point getGammaPerStep() const;
  OT::Point getCoefficientOfVariationPerStep() const;
  OT::Point getProbabilityEstimatePerStep() const;
  OT::Point getThresholdCoefficientOfVariationPerStep() const;

  /** Keep event sample */
  void setKeepEventSample(bool keepEventSample);

  /** Event input/output sample accessor */
  OT::Sample getEventInputSample() const;
  OT::Sample getEventOutputSample() const;

  /** All level sample accessor*/
  SampleCollection getOutputSample() const;
  SampleCollection getInputSample() const;

  /** i-subset */
  void setISubset(OT::Bool iSubset);
  void setBetaMin(OT::Scalar betaMin);

  /** Performs the actual computation. */
  void run() override;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

private:
  /** Result accessor */
  void setResult(const SubsetInverseSamplingResult & result);

  /** Compute the block sample */
  OT::Sample computeBlockSample() override;

  /** Compute the new threshold corresponding to the conditional failure probability */
  OT::Scalar computeThreshold();

  /** compute probability estimate on the current sample */
  OT::Scalar computeProbability(OT::Scalar probabilityEstimate, OT::Scalar threshold);

  /** Sort new seeds */
  void initializeSeed(OT::Scalar threshold);

  /** Compute the correlation on markov chains at the current state of the algorithm */
  OT::Scalar computeVarianceGamma(OT::Scalar currentFailureProbability, OT::Scalar threshold);

  /** Generate new points in the conditional failure domain */
  void generatePoints(OT::Scalar threshold);

  // The result
  SubsetInverseSamplingResult result_;

  // some parameters
  OT::Scalar proposalRange_;// width of the proposal pdf
  OT::Scalar conditionalProbability_;// conditional probability at each subset
  OT::Bool iSubset_;// conditional pre-sampling
  OT::Scalar betaMin_;// pre-sampling hypersphere exclusion radius
  OT::Bool keepEventSample_;// do we keep the event sample ?
  OT::Scalar targetProbability_;// final target probability

  // some results
  OT::UnsignedInteger numberOfSteps_;// number of subset steps
  OT::Point thresholdPerStep_;// intermediate thresholds
  OT::Point gammaPerStep_;// intermediate gammas
  OT::Point coefficientOfVariationPerStep_;// intermediate COVS
  OT::Point probabilityEstimatePerStep_;// intermediate PFs
  OT::Point thresholdCoefficientOfVariationPerStep_;// intermediate threshold COVs
  OT::Sample eventInputSample_;// event input sample
  OT::Sample eventOutputSample_;// event output sample
  SamplePersistentCollection allLevelSample_; // all event output sample
  SamplePersistentCollection allPointSample_; // all event output sample
  OT::Distribution thresholdDistribution_;//distribution of the final threshold value

  // attributes used for conveniency, not to be saved/loaded
  OT::StandardEvent standardEvent_;// the algorithm happens in U
  OT::UnsignedInteger dimension_;// input dimension
  OT::Sample currentPointSample_;// X
  OT::Sample currentLevelSample_;//f(X)

}; /* class SubsetInverseSampling */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_SUBSETINVERSESAMPLING_HXX */
