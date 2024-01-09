//                                               -*- C++ -*-
/**
 *  @brief SubsetSamplingImplementation
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
#include <openturns/PersistentObjectFactory.hxx>
#include "otrobopt/SubsetInverseSamplingResult.hxx"

using namespace OT;

namespace OTROBOPT
{

/*
 * @class SubsetInverseSamplingResult
 */

CLASSNAMEINIT(SubsetInverseSamplingResult);

static Factory<SubsetInverseSamplingResult> Factory_SubsetInverseSamplingResult;


/* Default constructor */
SubsetInverseSamplingResult::SubsetInverseSamplingResult()
: ProbabilitySimulationResult()
{
}


/* Constructor with parameters */
SubsetInverseSamplingResult::SubsetInverseSamplingResult(const RandomVector & event,
  const Scalar probabilityEstimate,
  const Scalar varianceEstimate,
  const UnsignedInteger outerSampling,
  const UnsignedInteger blockSize,
  const Scalar coefficientOfVariation,
  const Scalar threshold)
: ProbabilitySimulationResult(event, probabilityEstimate, varianceEstimate, outerSampling, blockSize),
  coefficientOfVariation_(coefficientOfVariation),
  threshold_(threshold)
{
}


/* Virtual constructor */
SubsetInverseSamplingResult * SubsetInverseSamplingResult::clone() const
{
  return new SubsetInverseSamplingResult(*this);
}


/* Coefficient of variation estimate accessor */
Scalar SubsetInverseSamplingResult::getCoefficientOfVariation() const
{
  return coefficientOfVariation_;
}

/* String converter */
String SubsetInverseSamplingResult::__repr__() const
{
  OSS oss;
  oss << ProbabilitySimulationResult::__repr__();
  oss << " threshold=" << threshold_;
  return oss;
}


/* Method save() stores the object through the StorageManager */
void SubsetInverseSamplingResult::save(Advocate & adv) const
{
  ProbabilitySimulationResult::save(adv);
  adv.saveAttribute("coefficientOfVariation_", coefficientOfVariation_);
}


/* Method load() reloads the object from the StorageManager */
void SubsetInverseSamplingResult::load(Advocate & adv)
{
  ProbabilitySimulationResult::load(adv);
  adv.loadAttribute("coefficientOfVariation_", coefficientOfVariation_);
}


} /* namespace OTROBOPT */
