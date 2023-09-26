//                                               -*- C++ -*-
/**
 *  @brief SubsetInverseSampling
 *
 *  Copyright 2005-2023 Airbus-EDF-IMACS-Phimeca
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

#include "otrobopt/SubsetInverseSampling.hxx"

#include <openturns/TruncatedDistribution.hxx>
#include <openturns/KernelSmoothing.hxx>
#include <openturns/Normal.hxx>
#include <openturns/ChiSquare.hxx>
#include <openturns/ComposedDistribution.hxx>
#include <openturns/SpecFunc.hxx>
#include <openturns/DistFunc.hxx>
#include <openturns/Uniform.hxx>
#include <openturns/RandomGenerator.hxx>
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/CompositeRandomVector.hxx>
#include <openturns/ThresholdEvent.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(SubsetInverseSampling);

static Factory<SubsetInverseSampling> Factory_SubsetInverseSampling;

const UnsignedInteger SubsetInverseSampling::DefaultMaximumOuterSampling = 10000;
const Scalar SubsetInverseSampling::DefaultConditionalProbability = 0.1;
const Scalar SubsetInverseSampling::DefaultProposalRange = 2.0;
const Scalar SubsetInverseSampling::DefaultBetaMin = 2.0;


/* Default constructor */
SubsetInverseSampling::SubsetInverseSampling()
: EventSimulation()
, proposalRange_(0.)
, conditionalProbability_(0.)
, iSubset_(false)
, betaMin_(0.)
, keepEventSample_(false)
, targetProbability_(0.)
, numberOfSteps_(0)
{
}


/* Constructor with parameters */
SubsetInverseSampling::SubsetInverseSampling(const RandomVector & event,
                               const Scalar targetProbability,
                               const Scalar proposalRange,
                               const Scalar conditionalProbability)
: EventSimulation(event)
, proposalRange_(proposalRange)
, conditionalProbability_(conditionalProbability)
, iSubset_(false)
, betaMin_(DefaultBetaMin)
, keepEventSample_(false)
, targetProbability_(targetProbability)
, numberOfSteps_(0)
{
  setMaximumOuterSampling( DefaultMaximumOuterSampling );// overide simulation default outersampling
  UnsignedInteger outputDimension = event.getFunction().getOutputDimension();
  if ( outputDimension > 1 )
    throw InvalidArgumentException(HERE) << "Output dimension for SubsetInverseSampling cannot be greater than 1, here output dimension=" << outputDimension;
}


/* Virtual constructor */
SubsetInverseSampling * SubsetInverseSampling::clone() const
{
  return new SubsetInverseSampling(*this);
}


/* Performs the actual computation. */
void SubsetInverseSampling::run()
{ 
  // First, initialize some parameters
  convergenceStrategy_.clear();
  numberOfSteps_ = 0;
  thresholdPerStep_.clear();
  thresholdCoefficientOfVariationPerStep_.clear();
  gammaPerStep_.clear();
  coefficientOfVariationPerStep_.clear();
  probabilityEstimatePerStep_.clear();
  eventInputSample_.clear();
  eventOutputSample_.clear();
  allPointSample_.clear();
  allLevelSample_.clear();

  dimension_ = getEvent().getAntecedent().getDimension();

  if ( getMaximumCoefficientOfVariation() != ResourceMap::GetAsScalar( "SimulationAlgorithm-DefaultMaximumCoefficientOfVariation" ) )
    Log::Warn(OSS() << "The maximum coefficient of variation was set. It won't be used as termination criteria.");

  if ( conditionalProbability_ * getMaximumOuterSampling() * getBlockSize() < 1 )
    throw InvalidArgumentException(HERE) << "The number of samples (maximumOuterSampling x blockSize : " << getMaximumOuterSampling() << " x " << getBlockSize() << ") should be >= " << ceil( 1.0 / conditionalProbability_ );
  
  if ( getMaximumOuterSampling() * getBlockSize() <= 100 )
    Log::Warn(OSS() << "The number of samples per step is very low : " << getMaximumOuterSampling()*getBlockSize() << ".");

  // perform isoprobabilistic transformation (the study is done in the standard space):
  standardEvent_ = StandardEvent(getEvent());

  Scalar currentCoVsquare = 0.0;
  Scalar varianceEstimate = 0.0;
  Scalar coefficientOfVariationSquare = 0.0;
  Scalar tempCoefficientOfVariation = 0.0;
  Scalar thresholdVariance = 0.0;
  Scalar thresholdCoefficientOfVariationSquare = 0.0;
  Scalar gamma = 0.0;

  // allocate input/output samples
  const UnsignedInteger maximumOuterSampling = getMaximumOuterSampling();
  const UnsignedInteger blockSize = getBlockSize();
  const UnsignedInteger N = maximumOuterSampling * blockSize;
  currentPointSample_ = Sample( N, dimension_ );
  currentLevelSample_ = Sample( N, getEvent().getFunction().getOutputDimension() );
  
  // Step 1: sampling
  for ( UnsignedInteger i = 0; i < maximumOuterSampling; ++ i )
  {
    Sample inputSample;
    if (!iSubset_) {
      // crude MC
      inputSample = standardEvent_.getAntecedent().getDistribution().getSample( blockSize );
    }
    else {
      // conditional sampling
      TruncatedDistribution truncatedChiSquare(ChiSquare(dimension_), betaMin_ * betaMin_, TruncatedDistribution::LOWER);
      Normal normal(dimension_);
      inputSample = Sample(0, dimension_);
      for ( UnsignedInteger j = 0; j < blockSize; ++ j)
      {
        Point direction = normal.getRealization();
        Scalar norm = direction.norm();
        Scalar radius = sqrt(truncatedChiSquare.getRealization()[0]);
        if (fabs(norm) > 1e-12)
        {
          radius *= 1.0 / norm;
        }
        inputSample.add(direction * radius);
      }
    }
    Sample blockSample( standardEvent_.getFunction()( inputSample ) );
    for ( UnsignedInteger j = 0 ; j < blockSize; ++ j )
    {
      currentPointSample_[ i*blockSize+j ] = inputSample[j];
      currentLevelSample_[ i*blockSize+j ] = blockSample[j];
    }
  }
  ++ numberOfSteps_;

  // Stop if the wanted probability is greater than the target probability per step 
  Bool stop = targetProbability_ >= conditionalProbability_;

  if  (stop)
  {
    setConditionalProbability(targetProbability_);
  }

  // computation of the first intermediate threshold with the sample create with a normal distribution */
  Scalar currentThreshold = computeThreshold(); 
  
  // compute monte carlo probability estimate
  Scalar probabilityEstimate = computeProbability(1.0, currentThreshold);
  
  if (iSubset_)
  {
    Scalar correction = 1.0 - ChiSquare(standardEvent_.getImplementation()->getAntecedent().getDistribution().getDimension()).computeCDF(betaMin_ * betaMin_);
    probabilityEstimate *= correction;
  }

  // compute the coefficient of variation
  if (probabilityEstimate > 0.0)
  {
    // ... compute coefficient of variation
    coefficientOfVariationSquare = (1.0 - probabilityEstimate) / (probabilityEstimate * currentLevelSample_.getSize() * 1.0);
    // ... compute variance estimate
    varianceEstimate = coefficientOfVariationSquare * probabilityEstimate * probabilityEstimate;

    // compute coefficient of variation of the threshold
    Distribution ks = KernelSmoothing().build(currentLevelSample_);
    thresholdVariance = probabilityEstimate * (1. - probabilityEstimate) / currentLevelSample_.getSize() / pow(ks.computePDF(currentThreshold), 2);
    thresholdCoefficientOfVariationSquare = thresholdVariance / (currentThreshold * currentThreshold);
  }
  
  allPointSample_.add(currentPointSample_);
  allLevelSample_.add(currentLevelSample_);
  thresholdPerStep_.add( currentThreshold );
  gammaPerStep_.add(0.);
  probabilityEstimatePerStep_.add(probabilityEstimate);
  coefficientOfVariationPerStep_.add(sqrt(coefficientOfVariationSquare));
  thresholdCoefficientOfVariationPerStep_.add(sqrt(thresholdCoefficientOfVariationSquare));
  
  // as long as the conditional failure domain do not overlap the global one
  while ( !stop )
  {    
    // prepare new seeds
    initializeSeed( currentThreshold );

    // create new points using MCMC
    generatePoints( currentThreshold );

    // save the level sample
    allPointSample_.add(currentPointSample_);
    allLevelSample_.add(currentLevelSample_);

    // compute new threshold
    currentThreshold = computeThreshold();

    // compute probability estimate on the current sample and group seeds at the beginning of the work sample
    Scalar currentProbabilityEstimate = computeProbability( probabilityEstimate, currentThreshold );
    
    // update probability estimate
    probabilityEstimate *= currentProbabilityEstimate;

    // update coefficient of variation 
    gamma = computeVarianceGamma( currentProbabilityEstimate, currentThreshold );
    currentCoVsquare = (1.0 - currentProbabilityEstimate) / (currentProbabilityEstimate * currentLevelSample_.getSize() * 1.0);
    tempCoefficientOfVariation = sqrt(coefficientOfVariationSquare + (1.0 + gamma) * currentCoVsquare);

    // update stopping criterion : target proba > inf bound of confidence interval of pf
    stop = targetProbability_ >= probabilityEstimate * (1 - tempCoefficientOfVariation * DistFunc::qNormal(0.95)) ;

    if (stop)
    {
      // change the target probability of the final step
      setConditionalProbability(targetProbability_ / probabilityEstimatePerStep_[numberOfSteps_-1]);
      // compute the final threshold
      currentThreshold = computeThreshold();
      // compute the current probability estimate 
      currentProbabilityEstimate = computeProbability(probabilityEstimatePerStep_[numberOfSteps_-1], currentThreshold);
      // update probability estimate
      probabilityEstimate = probabilityEstimatePerStep_[numberOfSteps_-1] * currentProbabilityEstimate;

      // update a second time the conditional probability to be as close as possible of the target probability
      // change the target probability of the final step
      setConditionalProbability(targetProbability_ * targetProbability_ / (probabilityEstimatePerStep_[numberOfSteps_-1] * probabilityEstimate));
      // compute the final threshold
      currentThreshold = computeThreshold();
      // compute the current probability estimate 
      currentProbabilityEstimate = computeProbability( probabilityEstimatePerStep_[numberOfSteps_-1], currentThreshold );
      // update probability estimate
      probabilityEstimate = probabilityEstimatePerStep_[numberOfSteps_-1] * currentProbabilityEstimate;
    }

    // update coefficient of variation 
    gamma = computeVarianceGamma(currentProbabilityEstimate, currentThreshold);
    currentCoVsquare = (1.0 - currentProbabilityEstimate) / (currentProbabilityEstimate * currentLevelSample_.getSize() * 1.0);
    coefficientOfVariationSquare += (1.0 + gamma) * currentCoVsquare;
    // update threshold coefficient of variation
    Distribution ks = KernelSmoothing().build(currentLevelSample_);
    thresholdVariance = currentProbabilityEstimate * (1. - currentProbabilityEstimate) / currentLevelSample_.getSize() / pow(ks.computePDF(currentThreshold), 2);
    thresholdCoefficientOfVariationSquare += (1.0 + gamma) * thresholdVariance / (currentThreshold * currentThreshold);

    thresholdPerStep_.add( currentThreshold );
    gammaPerStep_.add(gamma);
    probabilityEstimatePerStep_.add(probabilityEstimate);
    coefficientOfVariationPerStep_.add(sqrt(coefficientOfVariationSquare));
    thresholdCoefficientOfVariationPerStep_.add(sqrt(thresholdCoefficientOfVariationSquare));

    // stop if the number of subset steps is too high, else results are not numerically defined anymore
    if ( fabs( pow( probabilityEstimate, 2.) ) < SpecFunc::MinScalar )
      throw NotDefinedException(HERE) << "Probability estimate too small: " << probabilityEstimate;

    // compute variance estimate
    varianceEstimate = coefficientOfVariationSquare * pow( probabilityEstimate, 2. );
    thresholdVariance = thresholdCoefficientOfVariationSquare * pow( currentThreshold, 2. );

    ++ numberOfSteps_;

    if (stopCallback_.first && stopCallback_.first(stopCallback_.second))
      throw InternalException(HERE) << "User stopped simulation";
  }

  // define the threshold distribution
  thresholdDistribution_ = Normal(currentThreshold, sqrt(thresholdVariance));

  //update the event with the final threshold
  RandomVector modified_event = ThresholdEvent(CompositeRandomVector(getEvent().getFunction(), getEvent().getAntecedent()), getEvent().getOperator(), currentThreshold);

  setResult( SubsetInverseSamplingResult(modified_event, probabilityEstimate, varianceEstimate, numberOfSteps_ * getMaximumOuterSampling(), getBlockSize(), sqrt( coefficientOfVariationSquare ), currentThreshold) );

  // keep the event sample if requested
  if (keepEventSample_)
  {
    eventInputSample_ = Sample(0, dimension_);  
    eventOutputSample_ = Sample (0, getEvent().getFunction().getOutputDimension());
    for (UnsignedInteger i = 0; i < currentPointSample_.getSize(); ++ i)
    {
      if (getEvent().getOperator()(currentLevelSample_(i, 0), currentThreshold))
      {
        eventInputSample_.add(getEvent().getAntecedent().getDistribution().getInverseIsoProbabilisticTransformation()(currentPointSample_[i]));
        eventOutputSample_.add(currentLevelSample_[i]);
      }
    }
  }
  
  // free work samples
  currentLevelSample_.clear();
  currentPointSample_.clear();
}


/* Compute the block sample */
Sample SubsetInverseSampling::computeBlockSample()
{
  return Sample();
}


/* Compute the new threshold corresponding to the target failure probability */
Scalar SubsetInverseSampling::computeThreshold()
{
  // compute the quantile according to the event operator
  const Scalar ratio = getEvent().getOperator()(1.0, 2.0) ?  conditionalProbability_ : 1.0 - conditionalProbability_;
  
  const Scalar currentThreshold = currentLevelSample_.computeQuantile( ratio )[0];
  
  return currentThreshold;
}


Scalar SubsetInverseSampling::computeProbability(Scalar probabilityEstimateFactor, Scalar threshold)
{
  const UnsignedInteger maximumOuterSampling = getMaximumOuterSampling();
  const UnsignedInteger blockSize = getBlockSize();
  Scalar probabilityEstimate = 0.0;
  Scalar varianceEstimate = 0.0;
  
  for (UnsignedInteger i = 0; i < maximumOuterSampling; ++ i)
  {
    const Scalar size = i + 1.0;
    Scalar meanBlock = 0.0;
    Scalar varianceBlock = 0.0;
    for (UnsignedInteger j = 0 ; j < blockSize; ++ j)
    {
      if (getEvent().getOperator()( currentLevelSample_(i*blockSize+j, 0), threshold))
      {
        // update local mean and variance
        meanBlock += 1.0 / blockSize;
        varianceBlock += 1.0 * 1.0 / blockSize;
      }
    }
    varianceBlock -= pow( meanBlock, 2.0 );   
    
    // update global mean and variance
    varianceEstimate = (varianceBlock + (size - 1.0) * varianceEstimate) / size + (1.0 - 1.0 / size) * (probabilityEstimate - meanBlock) * (probabilityEstimate - meanBlock) / size;
    probabilityEstimate = std::min(1.0, (meanBlock + (size - 1.0) * probabilityEstimate) / size);
    
    // store convergence at each block
    Point convergencePoint(2);
    convergencePoint[0] = probabilityEstimate * probabilityEstimateFactor;
    convergencePoint[1] = varianceEstimate * probabilityEstimateFactor * probabilityEstimateFactor / size;
    convergenceStrategy_.store(convergencePoint);
  }
  
  // cannot determine next subset domain if no variance
  const Scalar epsilon = ResourceMap::GetAsScalar( "SpecFunc-Precision" );
  if ( fabs( varianceEstimate ) < epsilon )
    throw NotDefinedException(HERE) << "Null output variance";
                                          
  return probabilityEstimate;
}


/* Sort new seeds */
void SubsetInverseSampling::initializeSeed(Scalar threshold)
{
  UnsignedInteger seedIndex = 0;
  const UnsignedInteger maximumOuterSampling = getMaximumOuterSampling();
  const UnsignedInteger blockSize = getBlockSize();
  for ( UnsignedInteger i = 0; i < maximumOuterSampling; ++ i )
  {
    for ( UnsignedInteger j = 0 ; j < blockSize; ++ j )
    {
      if ( getEvent().getOperator()( currentLevelSample_[ i*blockSize+j ][0], threshold ) )
      {
        // initialize seeds : they're grouped at the beginning of the sample
        currentPointSample_[ seedIndex ] = currentPointSample_[ i*blockSize+j ];
        currentLevelSample_[ seedIndex ] = currentLevelSample_[ i*blockSize+j ];
        ++ seedIndex;
      }
    }
  }
}


/* Compute the correlation on markov chains at the current state of the algorithm */
Scalar SubsetInverseSampling::computeVarianceGamma(Scalar currentFailureProbability, Scalar threshold)
{
  const UnsignedInteger N = currentPointSample_.getSize();
  const UnsignedInteger Nc = std::max<UnsignedInteger>(1, conditionalProbability_ * N);
  Matrix IndicatorMatrice( Nc, N / Nc );
  Point correlationSequence( N / Nc - 1 );
  Scalar currentFailureProbability2 = pow( currentFailureProbability, 2. );
  for ( UnsignedInteger i = 0; i < N / Nc; ++ i )
  {
    for ( UnsignedInteger j = 0; j < Nc; ++ j )
    {
      IndicatorMatrice(j, i) = getEvent().getOperator()(currentLevelSample_[ i*Nc+j ][0], threshold);
    }
  }
  for ( UnsignedInteger k = 0; k < N / Nc - 1; ++ k )
  {
    for ( UnsignedInteger j = 0; j < Nc; ++ j )
    {
      for ( UnsignedInteger l = 0; l < N / Nc - k - 1; ++ l )
      {
        correlationSequence[k] += 1.0 * IndicatorMatrice(j, l) * IndicatorMatrice(j, l + (k + 1));
      }
    }
    correlationSequence[k] /= 1.0 * N - 1.0 * (k + 1) * Nc;
    correlationSequence[k] -= currentFailureProbability2;
  }
  const Scalar R0 = currentFailureProbability * ( 1.0 - currentFailureProbability );
  Point rho = ((1.0 / R0) * correlationSequence);
  Scalar gamma = 0.0;
  for ( UnsignedInteger k = 0; k < N / Nc - 1; ++ k )
  {
    gamma += 2.0 * (1.0 - (k + 1) * 1.0 * Nc / N) * rho[k];
  }
  return gamma;
}


/* Iterate one step of the algorithm */
void SubsetInverseSampling::generatePoints(Scalar threshold)
{  
  UnsignedInteger maximumOuterSampling = getMaximumOuterSampling();
  UnsignedInteger blockSize = getBlockSize();
  Distribution randomWalk(ComposedDistribution(ComposedDistribution::DistributionCollection(dimension_, Uniform(-0.5*proposalRange_, 0.5*proposalRange_))));
  UnsignedInteger N = currentPointSample_.getSize(); // total sample size
  UnsignedInteger Nc = conditionalProbability_ * N; //number of seeds (also = maximumOuterSampling*blockSize)
  
  for ( UnsignedInteger i = 0; i < maximumOuterSampling; ++ i )
  {    
    Sample inputSample( blockSize, dimension_ );
    for ( UnsignedInteger j = 0; j < blockSize; ++ j )
    {
      // assign the new point to the seed, seed points being regrouped at the beginning of the sample
      if ( i*blockSize+j >= Nc )
      {
        currentPointSample_[ i*blockSize+j ] = currentPointSample_[ i*blockSize+j-Nc ];
        currentLevelSample_[ i*blockSize+j ] = currentLevelSample_[ i*blockSize+j-Nc ];     
      }
      
      // generate a new point
      Point oldPoint( currentPointSample_[ i*blockSize+j ] );
      Point newPoint( oldPoint + randomWalk.getRealization() );
      
      // 1. accept / reject new components
      Point uniform( RandomGenerator::Generate(dimension_) );
      for (UnsignedInteger k = 0; k < dimension_; ++ k)
      {
        // compute ratio
        const Scalar ratio = std::min(1.0, exp(0.5 * (oldPoint[k] * oldPoint[k] - newPoint[k] * newPoint[k])));

        // accept new point with probability ratio
        if (ratio < uniform[k])
        {
          newPoint[k] = oldPoint[k];
        }
      }
      
      inputSample[j] = newPoint;
    }
   
    Sample blockSample( standardEvent_.getFunction()( inputSample ) );

    for ( UnsignedInteger j = 0; j < getBlockSize(); ++ j )
    {
      // 2. accept the new point if in the failure domain
      if ( getEvent().getOperator()( blockSample[j][0], threshold ) )
      {
        currentPointSample_[ i*blockSize+j ] = inputSample[j];
        currentLevelSample_[ i*blockSize+j ] = blockSample[j];
      }
    }
  }
}


/* Confidence Length of the threshold */
Scalar SubsetInverseSampling::getThresholdConfidenceLength(const Scalar level) const
{
  Scalar thresholdInf = thresholdDistribution_.computeQuantile((1 - level)/2)[0];
  Scalar thresholdSup = thresholdDistribution_.computeQuantile(1.-(1 - level)/2)[0];
  return Scalar (std::max(thresholdSup, thresholdInf) - std::min(thresholdSup, thresholdInf));
}


/* Markov parameter accessor */
void SubsetInverseSampling::setProposalRange(Scalar proposalRange)
{
  proposalRange_ = proposalRange;
}


Scalar SubsetInverseSampling::getProposalRange() const
{
  return proposalRange_;
}


/* Ratio accessor */
void SubsetInverseSampling::setConditionalProbability(Scalar conditionalProbability)
{
  if ( (conditionalProbability <= 0.) || (conditionalProbability >= 1.) ) throw InvalidArgumentException(HERE) << "In setConditionalProbability::Probability should be in (0, 1)";
  conditionalProbability_ = conditionalProbability;
}

Scalar SubsetInverseSampling::getConditionalProbability() const
{
  return conditionalProbability_;
}

/* target probability accessor */
void SubsetInverseSampling::setTargetProbability(Scalar targetProbability)
{
  if ( (targetProbability <= 0.) || (targetProbability >= 1.) ) throw InvalidArgumentException(HERE) << "In setTargetProbability::Probability should be in (0, 1)";
  targetProbability_ = targetProbability;
}

Scalar SubsetInverseSampling::getTargetProbability() const
{
  return targetProbability_;
}

UnsignedInteger SubsetInverseSampling::getNumberOfSteps()
{
  return numberOfSteps_;
}


OT::Point SubsetInverseSampling::getGammaPerStep() const
{
  return gammaPerStep_;
}


OT::Point SubsetInverseSampling::getCoefficientOfVariationPerStep() const
{
  return coefficientOfVariationPerStep_;
}


OT::Point SubsetInverseSampling::getProbabilityEstimatePerStep() const
{
  return probabilityEstimatePerStep_;
}


OT::Point SubsetInverseSampling::getThresholdPerStep() const
{
  return thresholdPerStep_;
}

OT::Point SubsetInverseSampling::getThresholdCoefficientOfVariationPerStep() const
{
  return thresholdCoefficientOfVariationPerStep_;
}

void SubsetInverseSampling::setKeepEventSample(bool keepEventSample)
{
  keepEventSample_ = keepEventSample;
}


Sample SubsetInverseSampling::getEventInputSample() const
{
  return eventInputSample_;
}


Sample SubsetInverseSampling::getEventOutputSample() const
{
  return eventOutputSample_;
}

SubsetInverseSampling::SampleCollection SubsetInverseSampling::getInputSample() const
{
  return allPointSample_;
}

SubsetInverseSampling::SampleCollection SubsetInverseSampling::getOutputSample() const
{
  return allLevelSample_;
}

void SubsetInverseSampling::setISubset(OT::Bool iSubset)
{
  iSubset_ = iSubset;
}

void SubsetInverseSampling::setBetaMin(Scalar betaMin)
{
  if (betaMin <= 0.) throw InvalidArgumentException(HERE) << "Beta min should be positive";
  betaMin_ = betaMin;
}

void SubsetInverseSampling::setResult(const SubsetInverseSamplingResult & result)
{
  result_ = result;
}

SubsetInverseSamplingResult SubsetInverseSampling::getResult() const
{
  return result_;
}

/* String converter */
String SubsetInverseSampling::__repr__() const
{
  OSS oss;
  oss << "class=" << getClassName()
      << " derived from " << EventSimulation::__repr__()
      << " event=" << event_
      << " targetProbability=" << targetProbability_
      << " proposalRange=" << proposalRange_
      << " conditionalProbability=" << conditionalProbability_
      << " keepEventSample_=" << keepEventSample_;
  return oss;
}


/* Method save() stores the object through the StorageManager */
void SubsetInverseSampling::save(Advocate & adv) const
{
  EventSimulation::save(adv);
  adv.saveAttribute("targetProbability", targetProbability_);
  adv.saveAttribute("proposalRange_", proposalRange_);
  adv.saveAttribute("conditionalProbability_", conditionalProbability_);
  adv.saveAttribute("iSubset_", iSubset_);
  adv.saveAttribute("betaMin_", betaMin_);
  adv.saveAttribute("keepEventSample_", keepEventSample_);  
  
  adv.saveAttribute("numberOfSteps_", numberOfSteps_);
  adv.saveAttribute("thresholdPerStep_", thresholdPerStep_);
  adv.saveAttribute("gammaPerStep_", gammaPerStep_);
  adv.saveAttribute("coefficientOfVariationPerStep_", coefficientOfVariationPerStep_);
  adv.saveAttribute("probabilityEstimatePerStep_", probabilityEstimatePerStep_);
}


/* Method load() reloads the object from the StorageManager */
void SubsetInverseSampling::load(Advocate & adv)
{
  EventSimulation::load(adv);
  adv.loadAttribute("targetProbability", targetProbability_);
  adv.loadAttribute("proposalRange_", proposalRange_);
  adv.loadAttribute("conditionalProbability_", conditionalProbability_);
  adv.loadAttribute("keepEventSample_", keepEventSample_);
  adv.loadAttribute("iSubset_", iSubset_);
  adv.loadAttribute("betaMin_", betaMin_);
  
  adv.loadAttribute("numberOfSteps_", numberOfSteps_);
  adv.loadAttribute("thresholdPerStep_", thresholdPerStep_);
  adv.loadAttribute("gammaPerStep_", gammaPerStep_);
  adv.loadAttribute("coefficientOfVariationPerStep_", coefficientOfVariationPerStep_);
  adv.loadAttribute("probabilityEstimatePerStep_", probabilityEstimatePerStep_);
}




} /* namespace OTROBOPT */
