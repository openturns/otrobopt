//                                               -*- C++ -*-
/**
 *  @brief Measure function evaluation implementation
 *
 *  Copyright 2005-2024 Airbus-EDF-IMACS-Phimeca
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
#ifndef OTROBOPT_MEASUREEVALUATIONIMPLEMENTATION_HXX
#define OTROBOPT_MEASUREEVALUATIONIMPLEMENTATION_HXX

#include "otrobopt/OTRobOptprivate.hxx"

#include <openturns/Distribution.hxx>
#include <openturns/IntegrationAlgorithm.hxx>

namespace OTROBOPT
{

/**
 * @class MeasureEvaluationImplementation
 *
 * Measure function evaluation implementation
 */
class OTROBOPT_API MeasureEvaluationImplementation
  : public OT::EvaluationImplementation
{
  CLASSNAME

public:
  /** Default constructor */
  MeasureEvaluationImplementation();

  /** Parameter constructor */
  MeasureEvaluationImplementation(const OT::Function & function,
                                  const OT::Distribution & distribution);

  /** Virtual constructor method */
  MeasureEvaluationImplementation * clone() const override;

  /** Evaluation */
  OT::Point operator()(const OT::Point & inP) const override;

  /** Distribution accessor */
  virtual void setDistribution(const OT::Distribution & distribution);
  virtual OT::Distribution getDistribution() const;

  /** Function accessor */
  virtual void setFunction(const OT::Function & function);
  virtual OT::Function getFunction() const;

  /** Dimension accessor */
  OT::UnsignedInteger getInputDimension() const override;
  OT::UnsignedInteger getOutputDimension() const override;

  /** Integration algorithm accessor */
  virtual void setIntegrationAlgorithm(const OT::IntegrationAlgorithm & algorithm);
  virtual OT::IntegrationAlgorithm getIntegrationAlgorithm() const;

  /** PDF threshold accessor */
  OT::Scalar getPDFThreshold() const;

  void setPDFThreshold(const OT::Scalar threshold);

  /** String converter */
  OT::String __repr__() const override;
  OT::String __str__(const OT::String & offset = "") const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

protected:
  /** Underlying parametric function */
  OT::Function function_;

  /** Parameter distribution of the emasure */
  OT::Distribution distribution_;

  /** Algorithm used to compute the high precision value of the measure */
  OT::IntegrationAlgorithm integrationAlgorithm_;

  /** Threshold under which PDF is consider to be zero */
  OT::Scalar pdfThreshold_;
}; /* class MeasureEvaluationImplementation */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_MEASUREEVALUATIONIMPLEMENTATION_HXX */
