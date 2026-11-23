//                                               -*- C++ -*-
/**
 *  @brief Parameter-aware distribution transformation
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
#ifndef OTROBOPT_PARAMETRICDISTRIBUTIONTRANSFORMATION_HXX
#define OTROBOPT_PARAMETRICDISTRIBUTIONTRANSFORMATION_HXX

#include <openturns/Description.hxx>
#include <openturns/Distribution.hxx>
#include <openturns/FunctionImplementation.hxx>
#include <openturns/Indices.hxx>
#include <openturns/Point.hxx>

#include "otrobopt/OTRobOptprivate.hxx"

namespace OTROBOPT
{

/**
 * @class ParametricDistributionTransformation
 *
 * Transformation from a fixed reference distribution to a
 * parameter-dependent distribution.
 *
 * The input is a point xi of the reference distribution, the output is a
 * point theta of the target distribution, and the parameter is the target
 * distribution parameter s = D_s.getParameter(). The transformation is
 * rebuilt from s whenever the parameter changes, so it does not rely on the
 * parameter vector of the underlying OpenTURNS transformation.
 */
class OTROBOPT_API ParametricDistributionTransformation
  : public OT::FunctionImplementation
{
  CLASSNAME

public:
  /** Default constructor */
  ParametricDistributionTransformation();

  /**
   * Parameter constructor with the automatic reference policy: the standard
   * distribution when it is parameter-free, otherwise the standard normal
   * distribution in the target dimension.
   */
  explicit ParametricDistributionTransformation(const OT::Distribution & distribution);

  /** Parameter constructor with an explicit reference distribution */
  ParametricDistributionTransformation(const OT::Distribution & distribution,
                                       const OT::Distribution & reference);

  /** Virtual constructor */
  ParametricDistributionTransformation * clone() const override;

  /** Underlying distribution (a copy) */
  OT::Distribution getDistribution() const;

  /** Reference distribution (a copy) */
  OT::Distribution getReferenceDistribution() const;

  /** Whether the standard fast path is used */
  OT::Bool usesStandardSpace() const;

  /** Evaluation */
  OT::Point operator()(const OT::Point & xi) const override;
  OT::Sample operator()(const OT::Sample & xi) const override;

  /** Gradient against the reference coordinates */
  OT::Matrix gradient(const OT::Point & xi) const override;

  /** Gradient against the distribution parameter */
  OT::Matrix parameterGradient(const OT::Point & xi) const override;

  /** Active parameter accessors: the parameter exposed to the optimizer */
  OT::Point getParameter() const override;
  void setParameter(const OT::Point & parameter) override;
  OT::Description getParameterDescription() const override;
  OT::UnsignedInteger getParameterDimension() const override;

  /** Full parameter accessors: active parameters and fixed or qualified ones */
  OT::Point getFullParameter() const;
  void setFullParameter(const OT::Point & parameter);
  OT::Description getFullParameterDescription() const;

  /** Active parameter indices in the full parameter vector */
  OT::Indices getActiveParameter() const;
  void setActiveParameter(const OT::Indices & active);

  /**
   * Default active parameter indices for a distribution.
   *
   * All parameters are active except the trailing dependence or copula
   * block when the distribution exposes it through
   * Distribution::getParametersCollection().  This is the policy used by the
   * default constructor; users can override it with setActiveParameter().
   */
  static OT::Indices GetDefaultActiveParameter(const OT::Distribution & distribution);

  /** Dimension accessors */
  OT::UnsignedInteger getInputDimension() const override;
  OT::UnsignedInteger getOutputDimension() const override;

  /** Description accessors */
  OT::Description getInputDescription() const override;
  OT::Description getOutputDescription() const override;

  /** The mutable cache is not thread-safe */
  OT::Bool isParallel() const override;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

private:
  static OT::Distribution cloneDistribution(const OT::Distribution & distribution);

  void initialize(const OT::Distribution & distribution,
                  const OT::Distribution & reference,
                  const OT::Bool hasReference);
  void configureReference(const OT::Distribution & reference,
                          const OT::Bool hasReference);
  void invalidateCache() const;
  void updateTransformation() const;
  void refreshEvaluationDelegates();

  OT::Distribution distribution_;
  OT::Distribution reference_;
  OT::Bool usesStandardSpace_ = false;
  OT::Indices activeParameter_;

  mutable OT::Function transformation_;
  mutable OT::Point cachedParameter_;
  mutable OT::Bool cacheValid_ = false;
}; /* class ParametricDistributionTransformation */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_PARAMETRICDISTRIBUTIONTRANSFORMATION_HXX */
