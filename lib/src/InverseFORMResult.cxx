//                                               -*- C++ -*-
/**
 *  @brief InverseFORMResult
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

#include "otrobopt/InverseFORM.hxx"
#include <openturns/Distribution.hxx>
#include <openturns/PersistentCollection.hxx>
#include <openturns/PersistentObjectFactory.hxx>

using namespace OT;

namespace OTROBOPT {

CLASSNAMEINIT(InverseFORMResult);

static Factory<InverseFORMResult> Factory_InverseFORMResult;

typedef PersistentCollection<PointWithDescription> PersistentSensitivity;

/*
 * @brief  Standard constructor: the class is defined by an optimisation algorithm, a failure event and a physical starting point
 */
InverseFORMResult::InverseFORMResult (const Point & standardSpaceDesignPoint,
                                      const RandomVector & event,
                                      const Bool isStandardPointOriginInFailureSpace,
                                      const Point & parameter,
                                      const Description & parameterDescription,
                                      const OT::Point & convergenceCriteriaOptimResult)
: FORMResult(standardSpaceDesignPoint, event, isStandardPointOriginInFailureSpace)
, parameter_(parameter)
, parameterDescription_(parameterDescription)
, convergenceCriteria_(convergenceCriteriaOptimResult)
{
}

/* Default constructor */
InverseFORMResult::InverseFORMResult()
: FORMResult()
{
  // Nothing to do
}

/* Virtual constructor */
InverseFORMResult * InverseFORMResult::clone() const
{
  return new InverseFORMResult(*this);
}


Description InverseFORMResult::getParameterDescription() const
{
  return parameterDescription_;
}


Point InverseFORMResult::getParameter() const
{
  return parameter_;
}


OT::Point InverseFORMResult::getConvergenceCriteria() const
{
  return convergenceCriteria_;
}


/* String converter */
String InverseFORMResult::__repr__() const
{
  OSS oss;
  oss << "class=" << InverseFORMResult::GetClassName()
      << " " << FORMResult::__repr__()
      << " parameter_=" << parameter_
      << " parameterDescription_=" << parameterDescription_
      << " convergenceCriteria=" << convergenceCriteria_;
  return oss;
}

/* Method save() stores the object through the StorageManager */
void InverseFORMResult::save(Advocate & adv) const
{
  FORMResult::save(adv);
  adv.saveAttribute( "parameter_", parameter_ );
  adv.saveAttribute( "parameterDescription_", parameterDescription_ );
  adv.saveAttribute( "convergenceCriteria_", convergenceCriteria_);
}

/* Method load() reloads the object from the StorageManager */
void InverseFORMResult::load(Advocate & adv)
{
  FORMResult::load(adv);
  adv.loadAttribute( "parameter_", parameter_ );
  adv.loadAttribute( "parameterDescription_", parameterDescription_ );
  adv.loadAttribute( "convergenceCriteria_", convergenceCriteria_);
}

}
