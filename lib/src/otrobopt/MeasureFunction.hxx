//                                               -*- C++ -*-
/**
 *  @brief MeasureFunction
 *
 *  Copyright 2005-2015 Airbus-EDF-IMACS-Phimeca
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
#ifndef OTROBOPT_MEASUREFUNCTION_HXX
#define OTROBOPT_MEASUREFUNCTION_HXX

#include <openturns/TypedInterfaceObject.hxx>
#include <openturns/StorageManager.hxx>
#include <openturns/NumericalPoint.hxx>
#include "otrobopt/OTRobOptprivate.hxx"

namespace OTROBOPT
{

/* forward declaration */
class MeasureFunctionImplementation;

/**
 * @class MeasureFunction
 *
 * MeasureFunction is some measurefunction type to illustrate how to add some classes in Open TURNS
 */
class OTROBOPT_API MeasureFunction
  : public OT::TypedInterfaceObject<MeasureFunctionImplementation>
{
  CLASSNAME;

public:

  /** Default constructor */
  MeasureFunction();

  /** a func that return a point squared. **/
  OT::NumericalPoint square(OT::NumericalPoint& p) const;

  /** String converter */
  OT::String __repr__() const;

private:

}; /* class MeasureFunction */

} /* namespace OTROBOPT */

#endif /* OTROBOPT_MEASUREFUNCTION_HXX */
