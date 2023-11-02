//                                               -*- C++ -*-
/**
 *  @brief SubsetInverseSamplingResult
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
#ifndef OTROBOPT_SUBSETINVERSESAMPLINGRESULT_HXX
#define OTROBOPT_SUBSETINVERSESAMPLINGRESULT_HXX

#include <openturns/ProbabilitySimulationResult.hxx>
#include "otrobopt/OTRobOptprivate.hxx"

namespace OTROBOPT
{

class OTROBOPT_API SubsetInverseSamplingResult
: public OT::ProbabilitySimulationResult
{
  CLASSNAME
public:
  
  /** Constructor with parameters */
  SubsetInverseSamplingResult();

  /** Constructor with parameters */
  SubsetInverseSamplingResult(const OT::RandomVector & event,
                       const OT::Scalar probabilityEstimate,
                       const OT::Scalar varianceEstimate,
                       const OT::UnsignedInteger outerSampling,
                       const OT::UnsignedInteger blockSize,
                       const OT::Scalar coefficientOfVariation = 0.0,
                       const OT::Scalar threshold = 0.0);

  /** Virtual constructor */
  SubsetInverseSamplingResult * clone() const override;

  /** Coefficient of variation estimate accessor */
  OT::Scalar getCoefficientOfVariation() const override;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

protected:
  OT::Scalar coefficientOfVariation_;
  OT::Scalar threshold_;

private:

}; /* class SubsetInverseSamplingResult */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_SUBSETINVERSESAMPLINGRESULT_HXX */
