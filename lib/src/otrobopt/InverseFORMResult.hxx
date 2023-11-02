//                                               -*- C++ -*-
/**
 *  @brief InverseFORMResult
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
#ifndef OTROBOPT_INVERSEFORMRESULT_HXX
#define OTROBOPT_INVERSEFORMRESULT_HXX

#include <openturns/FORMResult.hxx>
#include "otrobopt/OTRobOptprivate.hxx"

namespace OTROBOPT {


/**
 * @class InverseFORMResult
 */
class OTROBOPT_API InverseFORMResult:
  public OT::FORMResult
{
  CLASSNAME
public:

  /** Standard constructor */
  InverseFORMResult(const OT::Point & standardSpaceDesignPoint,
                    const OT::RandomVector & event,
                    const OT::Bool isStandardPointOriginInFailureSpace,
                    const OT::Point & parameter,
                    const OT::Description & parameterDescription,
                    const OT::Point & convergenceCriteria);

  /* Default constructor */
  InverseFORMResult();

  /** Virtual constructor */
  virtual InverseFORMResult * clone() const override;

  /** String converter */
  OT::String __repr__() const override;

  /** Method save() stores the object through the StorageManager */
  void save(OT::Advocate & adv) const override;

  /** Method load() reloads the object from the StorageManager */
  void load(OT::Advocate & adv) override;

  // parameter accessor
  OT::Point getParameter() const;
  OT::Description getParameterDescription() const;

  OT::Point getConvergenceCriteria() const;

protected:
  OT::Point parameter_;
  OT::Description parameterDescription_;
  OT::Point convergenceCriteria_;
};

}

#endif



