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
#include "otrobopt/MeasureFunction.hxx"
#include "otrobopt/MeasureFunctionImplementation.hxx"
#include <openturns/PersistentObjectFactory.hxx>

using namespace OT;

namespace OTROBOPT
{

CLASSNAMEINIT(MeasureFunction);

/* Default constructor */
MeasureFunction::MeasureFunction()
  : TypedInterfaceObject<MeasureFunctionImplementation>(new MeasureFunctionImplementation)
{
  // Nothing to do
}

NumericalPoint MeasureFunction::square(NumericalPoint& p) const
{
  return getImplementation()->square(p);
}

/* String converter */
String MeasureFunction::__repr__() const
{
  OSS oss;
  oss << "class=" << MeasureFunction::GetClassName()
      << " implementation=" << getImplementation()->__repr__();
  return oss;
}


} /* namespace OTROBOPT */
