//                                               -*- C++ -*-
/**
 *  @brief Measure gradient implementation
 *
 *  Copyright 2005-2026 Airbus-EDF-IMACS-ONERA-Phimeca
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
#ifndef OTROBOPT_MEASUREGRADIENT_HXX
#define OTROBOPT_MEASUREGRADIENT_HXX

#include "otrobopt/OTRobOptprivate.hxx"
#include "otrobopt/MeasureEvaluation.hxx"

#include <openturns/GradientImplementation.hxx>

namespace OTROBOPT
{

/**
 * @class MeasureGradient
 *
 * Gradient of a measure with respect to its input.
 *
 * A measure of the form
 * @f[
 *   M(x) = \int f(x;\theta)\,\pi(\theta)\,d\theta
 * @f]
 * is differentiated under the integral sign,
 * @f[
 *   D_x M(x) = \int D_x f(x;\theta)\,\pi(\theta)\,d\theta,
 * @f]
 * where the derivative of the integrand is supplied by the measure
 * implementation through MeasureEvaluationImplementation::integrandGradient.
 * The integration follows the measure evaluation: the same integration
 * algorithm, probability density threshold and discrete support are used.
 *
 * An aggregated measure concatenates the gradients of its components instead
 * of integrating a single integrand, so that the two modes share the same
 * gradient class.
 */
class OTROBOPT_API MeasureGradient
  : public OT::GradientImplementation
{
  CLASSNAME

public:
  typedef OT::Collection<MeasureEvaluation>           MeasureEvaluationCollection;
  typedef OT::PersistentCollection<MeasureEvaluation> MeasureEvaluationPersistentCollection;

  /** Default constructor */
  MeasureGradient();

  /** Constructor from a measure evaluation */
  explicit MeasureGradient(const MeasureEvaluation & evaluation);

  /** Constructor from the component measures of an aggregate */
  explicit MeasureGradient(const MeasureEvaluationCollection & collection);

  /** Virtual constructor method */
  MeasureGradient * clone() const override;

  /** Gradient */
  OT::Matrix gradient(const OT::Point & inP) const override;

  /** Dimension accessors */
  OT::UnsignedInteger getInputDimension() const override;
  OT::UnsignedInteger getOutputDimension() const override;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

private:

  /** Rebuild the component gradients of an aggregate */
  void buildComponentGradients();

  /** Measure evaluation, used in integrand mode */
  MeasureEvaluation evaluation_;

  /** Component measures, used in aggregate mode */
  MeasureEvaluationPersistentCollection collection_;

  /** Component gradients, used in aggregate mode */
  OT::Collection<OT::Gradient> componentGradients_;

}; /* class MeasureGradient */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_MEASUREGRADIENT_HXX */
