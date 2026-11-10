//                                               -*- C++ -*-
/**
 *  @brief Canonicalization of robust optimization problems
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
#ifndef OTROBOPT_ROBUSTOPTIMIZATIONPROBLEMCANONICALIZER_HXX
#define OTROBOPT_ROBUSTOPTIMIZATIONPROBLEMCANONICALIZER_HXX

#include <openturns/Distribution.hxx>
#include <openturns/Function.hxx>
#include <openturns/Interval.hxx>
#include <openturns/Point.hxx>

#include "otrobopt/OTRobOptprivate.hxx"
#include "otrobopt/RobustOptimizationProblem.hxx"

namespace OTROBOPT
{

/**
 * @class CanonicalizationOptions
 *
 * Options of the robust problem canonicalizer.
 *
 * The worst-case policy selects both the reference distribution and the
 * treatment of the worst-case optimization domain:
 *
 * - AUTO: use the explicit reference distribution when it is set, otherwise
 *   the standard fast path when it is available, otherwise the standard
 *   normal distribution.  For a worst-case measure, an explicit domain is
 *   required unless the standard fast path is used or the reference has a
 *   bounded range.
 * - STANDARD: require the standard fast path.
 * - NORMAL: force the standard normal reference.
 * - USER_REFERENCE: require the reference distribution.
 * - USER_DOMAIN: use the automatic reference policy and require the
 *   worst-case domain.
 */
class OTROBOPT_API CanonicalizationOptions
{
public:
  enum WorstCasePolicy
  {
    AUTO,
    STANDARD,
    NORMAL,
    USER_REFERENCE,
    USER_DOMAIN
  };

  /** Default constructor */
  CanonicalizationOptions();

  /** Reference distribution, empty for the automatic policy */
  OT::Distribution reference;

  /** Bounds on the distribution parameter, empty for no bounds */
  OT::Interval parameterBounds;

  /** Initial distribution parameter, empty for the current parameter */
  OT::Point parameterStart;

  /** Whether the distribution parameter is part of the decision vector */
  OT::Bool optimizeParameter = true;

  /** Worst-case policy */
  WorstCasePolicy worstCasePolicy = AUTO;

  /** Worst-case optimization domain in the reference space, empty for the
      range of the reference distribution */
  OT::Interval worstCaseDomain;

  /** Reference distribution accessors */
  void setReference(const OT::Distribution & reference);
  OT::Distribution getReference() const;
  void clearReference();
  OT::Bool hasReference() const;

  /** Parameter bounds accessors */
  void setParameterBounds(const OT::Interval & bounds);
  OT::Interval getParameterBounds() const;

  /** Parameter start accessors */
  void setParameterStart(const OT::Point & start);
  OT::Point getParameterStart() const;

  /** Parameter optimization accessors */
  void setOptimizeParameter(OT::Bool optimizeParameter);
  OT::Bool getOptimizeParameter() const;

  /** Worst-case policy accessors */
  void setWorstCasePolicy(WorstCasePolicy policy);
  WorstCasePolicy getWorstCasePolicy() const;

  /** Worst-case domain accessors */
  void setWorstCaseDomain(const OT::Interval & domain);
  OT::Interval getWorstCaseDomain() const;

  /** String converter */
  OT::String __repr__() const;

private:
  OT::Bool hasReference_ = false;
}; /* class CanonicalizationOptions */

/**
 * @class RobustOptimizationProblemCanonicalizer
 *
 * Canonicalize a robust optimization problem over a fixed reference
 * distribution.
 *
 * The canonical decision vector is y = (x, s) where x is the design vector
 * and s is the parameter of the distribution D_s.  The measure distribution
 * becomes a fixed reference distribution Xi and the parametric functions are
 * rebuilt as P(x, s; xi) = f(x; T_s(xi)).  When optimizeParameter is false,
 * the distribution parameter is frozen and the canonical decision vector is
 * x only.
 */
class OTROBOPT_API RobustOptimizationProblemCanonicalizer
{
public:
  /** Parameter constructor */
  explicit RobustOptimizationProblemCanonicalizer(
    const RobustOptimizationProblem & problem,
    const CanonicalizationOptions & options = CanonicalizationOptions());

  /** Canonical problem accessor */
  RobustOptimizationProblem getCanonicalProblem() const;

  /** Reference distribution accessor */
  OT::Distribution getReferenceDistribution() const;

  /** Canonical objective function accessor */
  OT::Function getCanonicalObjectiveFunction() const;

  /** Canonical inequality constraint function accessor */
  OT::Function getCanonicalConstraintFunction() const;

  /** Canonical starting point accessor */
  OT::Point getCanonicalStartingPoint() const;

  /** Whether the distribution parameter is optimized */
  OT::Bool getOptimizeParameter() const;

  /** Whether the standard fast path is used */
  OT::Bool usesStandardSpace() const;

  /** String converter */
  OT::String __repr__() const;

private:
  void initialize(const RobustOptimizationProblem & problem,
                  const CanonicalizationOptions & options);

  OT::Bool isStandardReference(const OT::Distribution & distribution) const;

  OT::Distribution selectReference(const OT::Distribution & distribution,
                                   const CanonicalizationOptions & options,
                                   OT::Bool & usesStandardSpace) const;

  OT::Function buildCanonicalFunction(const OT::Function & function,
                                      const OT::Function & transformation,
                                      const OT::UnsignedInteger designDimension,
                                      const OT::UnsignedInteger parameterDimension,
                                      const OT::UnsignedInteger referenceDimension) const;

  OT::Function liftFunction(const OT::Function & function,
                            const OT::UnsignedInteger designDimension,
                            const OT::UnsignedInteger parameterDimension) const;

  MeasureEvaluation canonicalizeMeasure(
    const MeasureEvaluation & measure,
    const OT::Distribution & distribution,
    const OT::Function & transformation,
    const OT::UnsignedInteger designDimension,
    const OT::UnsignedInteger parameterDimension,
    const OT::UnsignedInteger referenceDimension,
    const OT::Interval & optimizationDomain,
    const OT::Bool optimizeParameter) const;

  void checkMeasureDistribution(const MeasureEvaluation & measure,
                                const OT::Distribution & distribution,
                                const OT::String & role) const;

  OT::Interval buildBounds(const OT::Interval & designBounds,
                           const OT::Interval & parameterBounds,
                           const OT::UnsignedInteger designDimension,
                           const OT::UnsignedInteger parameterDimension,
                           const OT::Bool optimizeParameter) const;

  OT::Point buildStartingPoint(const OT::Interval & designBounds,
                               const OT::Point & parameterStart,
                               const OT::UnsignedInteger designDimension,
                               const OT::UnsignedInteger parameterDimension,
                               const OT::Bool optimizeParameter) const;

  OT::Point referenceStartingPoint(const OT::Distribution & reference) const;

  RobustOptimizationProblem canonicalProblem_;
  OT::Distribution reference_;
  OT::Point startingPoint_;
  OT::Bool optimizeParameter_ = true;
  OT::Bool usesStandardSpace_ = false;
}; /* class RobustOptimizationProblemCanonicalizer */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_ROBUSTOPTIMIZATIONPROBLEMCANONICALIZER_HXX */
