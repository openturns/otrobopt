//                                               -*- C++ -*-
/**
 *  @brief Quadrature estimator of a system-event probability
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
#ifndef OTROBOPT_QUADRATUREESTIMATOR_HXX
#define OTROBOPT_QUADRATUREESTIMATOR_HXX

#include "otrobopt/ProbabilityEstimatorImplementation.hxx"
#include <openturns/IntegrationAlgorithm.hxx>

namespace OTROBOPT
{

/**
 * @class QuadratureEstimator
 *
 * Deterministic estimator of a system-event probability.
 *
 * For a continuous distribution D, the probability is the Lebesgue integral
 * of p(theta) times the Boolean predicate evaluated on the indicators of the
 * limit-state components, over the range of D.  The density threshold only
 * truncates the tails of this continuous integrand.
 *
 * For a discrete distribution, the probability is the exact sum over the
 * support: no threshold is applied, because dropping an atom changes the
 * probability instead of truncating a tail.
 */
class OTROBOPT_API QuadratureEstimator
  : public ProbabilityEstimatorImplementation
{
  CLASSNAME

public:
  /** Default constructor */
  QuadratureEstimator();

  /** Constructor from an integration algorithm */
  explicit QuadratureEstimator(const OT::IntegrationAlgorithm & algorithm);

  /** Virtual constructor method */
  QuadratureEstimator * clone() const override;

  /** Integration algorithm accessor */
  void setIntegrationAlgorithm(const OT::IntegrationAlgorithm & algorithm);
  OT::IntegrationAlgorithm getIntegrationAlgorithm() const;

  /** Density threshold below which the continuous integrand is zero */
  void setPDFThreshold(OT::Scalar threshold);
  OT::Scalar getPDFThreshold() const;

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

  /** Error implementation: no quadrature gap is available */
  OT::Scalar computeEstimationError(const OT::Function & predicate,
                                    const OT::Function & limitState,
                                    const OT::Distribution & distribution,
                                    const OT::Point & designPoint) const override;

private:
  OT::IntegrationAlgorithm integrationAlgorithm_;
  OT::Scalar pdfThreshold_;

}; /* class QuadratureEstimator */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_QUADRATUREESTIMATOR_HXX */
