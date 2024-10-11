//                                               -*- C++ -*-
/**
 *  @brief InverseFORM
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
#include <openturns/PersistentObjectFactory.hxx>
#include <openturns/Normal.hxx>
#include <openturns/StandardEvent.hxx>
#include <openturns/LinearFunction.hxx>
#include <openturns/JointDistribution.hxx>
#include <openturns/CompositeRandomVector.hxx>
#include <openturns/ThresholdEvent.hxx>
#include <openturns/ComposedFunction.hxx>
#include <openturns/SpecFunc.hxx>

#if OPENTURNS_VERSION >= 102400
#include <openturns/DeconditionedDistribution.hxx>
#else
#include <openturns/ConditionalDistribution.hxx>
#define DeconditionedDistribution ConditionalDistribution
#endif


using namespace OT;

namespace OTROBOPT {


CLASSNAMEINIT(InverseFORM);

static Factory< InverseFORM > Factory_InverseFORM;

/* Default constructor for the save/load mechanism */
InverseFORM::InverseFORM()
: PersistentObject()
, targetBeta_(-Normal().computeQuantile(1e-2)[0])
{
  // Nothing to do
}

/*
 * @brief  Standard constructor: the class is defined by an optimisation algorithm, a failure event and a physical starting point
 */
InverseFORM::InverseFORM(const RandomVector & event,
                         const String & parameterName,
           const Point & physicalStartingPoint)
: PersistentObject()
, event_(event)
, parameterName_(parameterName)
, physicalStartingPoint_(physicalStartingPoint)
, targetBeta_(-Normal().computeQuantile(1e-2)[0])
{
  const Description parameterDescription(event_.getImplementation()->getFunction().getParameterDescription());
  parameterIndex_ = parameterDescription.find(parameterName);
  if (parameterIndex_ >= parameterDescription.getSize())
    throw InvalidArgumentException(HERE) << "Cannot find parameter " << parameterName;
}

/* Virtual constructor */
InverseFORM * InverseFORM::clone() const
{
  return new InverseFORM(*this);
}

/* Result accessor */
InverseFORMResult InverseFORM::getResult() const
{
  return result_;
}

/* Result accessor */
void InverseFORM::setResult(const InverseFORMResult & formResult)
{
  result_ = formResult;
}


/* String converter */
String InverseFORM::__repr__() const
{
  OSS oss;
  oss << "class=" << InverseFORM::GetClassName()
      << " result=" << result_;
  return oss;
}


/* Merit function used to optimize the step */
Scalar InverseFORM::meritFunction(const Point & u, const Scalar levelValue) const
{
  if (fixedStep_)
    return -1.0;

  const Scalar unormSquare = u.normSquare();
  return 0.5*unormSquare + (targetBeta_*targetBeta_*unormSquare*1.0 / (variableConvergence_*variableConvergence_) + 1000.0)*fabs(levelValue);
}


/* Function that computes the design point by re-using the Analytical::run() and creates a InverseFORMResult */
void InverseFORM::run()
{
  result_ = InverseFORMResult();

  const Point u0 = event_.getImplementation()->getAntecedent().getDistribution().getIsoProbabilisticTransformation().operator()(physicalStartingPoint_);

  // sign of target beta
  const Scalar signBetaT = targetBeta_ < 0. ? -1.0: 1.0;

  Point u = u0;
  Point unew;
  Point du;

  Scalar p = event_.getImplementation()->getFunction().getParameter()[parameterIndex_];
  Function G(getG(p));
  Scalar g = G(u0)[0];

  Scalar g0 = g;

  Scalar gnew = -1.0;
  Point dgdu;
  Scalar pnew = -1.0;
  Scalar dgdp = -1.0;
  Scalar dp = -1.0;
  const Scalar eps = ResourceMap::GetAsScalar("CenteredFiniteDifferenceGradient-DefaultEpsilon");
  Scalar m = 0.;
  Scalar mnew = -1.0;
  Scalar beta = signBetaT * u.norm();

  const Scalar betaC = targetBeta_;
  Bool convergence = false;
  UnsignedInteger iteration = 0;
  while ((!convergence) && (iteration < maximumIteration_) && (!stop_))
  {
    // some initializations
    event_.getImplementation()->getFunction().getImplementation()->setParameter(Point(1, p));
    m = meritFunction(u, g);

    // gradient / point
    dgdu = G.gradient(u) * Point({1.0});
    Scalar dgdunorm = dgdu.norm();

    // gradient / parameter
    Scalar rel = fabs(eps * p);
    Scalar delta = rel > SpecFunc::ScalarEpsilon ? rel : eps;

    G = getG(p + delta);
    Scalar right = G(u)[0];
    G = getG(p - delta);
    Scalar left = G(u)[0];

    G = getG(p);
    dgdp = (right - left) / (2.0 * delta);

    // direction
    du = -(betaC / dgdunorm) * dgdu - u;
    dp = (fabs(dgdp) < (SpecFunc::ScalarEpsilon * SpecFunc::ScalarEpsilon)) ? 0.0 : (u.dot(dgdu) - g + betaC * dgdunorm) / dgdp;
    LOGINFO(OSS() << "InverseFORM::run i=" << iteration << " u="<<u.__str__() <<" dgdu="<<dgdu.__str__()<<" du="<<du<<" dgdp="<< dgdp<< " dp="<<dp);
    
    // update according to the step
    Scalar step = fixedStep_ ? fixedStepValue_ : 1.0;
    UnsignedInteger stepIteration = 0;
    do
    {
      unew = u + step * du;

      pnew = p + step * dp;
      G = getG(pnew);
      gnew = G(unew)[0];
      mnew = meritFunction(unew, gnew);
      step *= 0.5;
      ++ stepIteration;
      LOGINFO(OSS() << "InverseFORM::run i=" << iteration << " step=" << step<< "pnew="<<pnew<<" unew="<<unew);
    }
    while ((!fixedStep_) && (mnew > m) && (stepIteration < variableStepMaxIterations_));

    // test for convergence
    convergence = ( fabs(pnew - p) < variableConvergence_)
                &&( fabs(gnew) < limitStateConvergence_)
                &&( fabs(unew.norm()-fabs(betaC)) < betaConvergence_);

    LOGINFO(OSS() << "InverseFORM::run i=" << iteration << " var=" << p <<" beta="<<beta<<" g/go="<<g/g0);
    LOGINFO(OSS() << "InverseFORM::run i=" << iteration << " convergence.var=" << fabs(pnew - p) <<" <? "<<variableConvergence_);
    LOGINFO(OSS() << "InverseFORM::run i=" << iteration << " convergence.g=" << fabs(gnew) <<" <? "<<limitStateConvergence_);
    LOGINFO(OSS() << "InverseFORM::run i=" << iteration << " convergence.beta=" <<fabs(unew.norm()-fabs(betaC)) <<" <? "<<betaConvergence_);

    // for next iteration
    m = mnew;
    u = unew;
    p = pnew;
    g = gnew;

    beta = signBetaT*u.norm();
    LOGINFO(OSS() << "InverseFORM::run i="<<iteration<<" g="<<g<<" du="<<du<<" u="<<u<<" beta="<<betaC<<" dp="<<dp<<" p="<<p<< "m="<<m<<" dgdp="<<dgdp<<" dgdu="<<dgdu<<" dgdunorm="<<dgdunorm<<" rel="<<rel<<" delta="<<delta);

    ++ iteration;
  }

  // convergence criteria result
  const Point criteria = {(Scalar)iteration, fabs(pnew - p), fabs(gnew), fabs(unew.norm()-fabs(betaC))};

  // standard space origin in failure space ?
  const Point origin(u.getDimension(), 0.0);
  const Point value(event_.getFunction()(event_.getImplementation()->getAntecedent().getDistribution().getInverseIsoProbabilisticTransformation().operator()(origin)));
  const Bool isStandardPointOriginInFailureSpace = event_.getOperator().compare(value[0], event_.getThreshold());

  // save some results
  result_ = InverseFORMResult(u, event_, isStandardPointOriginInFailureSpace, Point({p}), event_.getImplementation()->getFunction().getParameterDescription(), criteria);
}


Function InverseFORM::getG(const Scalar p)
{
  const Scalar threshold = event_.getThreshold();
  const ComparisonOperator op(event_.getOperator());
  Function newFunction(event_.getImplementation()->getFunction());

  PointWithDescription params(event_.getImplementation()->getFunction().getParameter());
  params[parameterIndex_] = p;
  newFunction.setParameter(params);
  RandomVector antecedent(event_.getImplementation()->getAntecedent().getImplementation()->clone());
  const Distribution distribution(antecedent.getDistribution());
  const JointDistribution * p_joint = dynamic_cast<JointDistribution *>(distribution.getImplementation().get());
  if (p_joint)
  {
    JointDistribution::DistributionCollection distributionCollection(p_joint->getDistributionCollection());
    for (UnsignedInteger i = 0; i < distributionCollection.getSize(); ++ i)
    {
      if (Description({"DeconditionedDistribution", "ConditionalDistribution"}).contains(distributionCollection[i].getImplementation()->getClassName()))
      {
        DistributionImplementation::PointWithDescriptionCollection parametersCollection(distributionCollection[i].getParametersCollection());
        for (UnsignedInteger j = 0; j < parametersCollection.getSize(); ++ j)
        {
          const String marginalName(parametersCollection[j].getName());
          const size_t pos = marginalName.find(parameterName_);
          if (pos != String::npos) {
            const String endName = marginalName.substr(pos, marginalName.size());
            if (endName == parameterName_)
            {
              parametersCollection[j] = params;
            }
          }
        }
        const DeconditionedDistribution * p_conditional = dynamic_cast<DeconditionedDistribution
        *>(distributionCollection[i].getImplementation().get());
        if (p_conditional)
        {
          Distribution conditioning(p_conditional->getConditioningDistribution());
          conditioning.setParametersCollection(parametersCollection);
          DeconditionedDistribution newConditional(p_conditional->getConditionedDistribution(), conditioning);
          distributionCollection[i] = newConditional;
          JointDistribution newDistribution(distributionCollection);
          antecedent = RandomVector(newDistribution);
        } // if p_conditional
      } // if conditional
    } // i
  } // if p_joint

  const CompositeRandomVector composite(newFunction, antecedent);
  const ThresholdEvent event(composite, op, threshold);
  const StandardEvent standardEvent(event);
  const Scalar gsign = op(1.0, 2.0) ? 1.0 : -1.0;
  const LinearFunction handleOp(Point(1), Point(1, -1.0*gsign*threshold), gsign*IdentityMatrix(1));
  const ComposedFunction composed(handleOp, standardEvent.getImplementation()->getFunction());
  return composed;
}


void InverseFORM::setTargetBeta(Scalar targetBeta) { targetBeta_ = targetBeta; }
Scalar InverseFORM::getTargetBeta() const { return targetBeta_; }

void InverseFORM::setFixedStep(Bool fixedStep) { fixedStep_ = fixedStep; }
Bool InverseFORM::getFixedStep() const { return fixedStep_; }

void InverseFORM::setFixedStepValue(Scalar fixedStepValue) { fixedStepValue_ = fixedStepValue; }
Scalar InverseFORM::getFixedStepValue() const { return fixedStepValue_; }

void InverseFORM::setVariableStepMaxIterations(UnsignedInteger variableStepMaxIterations) { variableStepMaxIterations_ = variableStepMaxIterations; }
UnsignedInteger InverseFORM::getVariableStepMaxIterations() const { return variableStepMaxIterations_; }

void InverseFORM::setMaximumIteration(UnsignedInteger maximumIteration) { maximumIteration_ = maximumIteration; }
UnsignedInteger InverseFORM::getMaximumIteration() const { return maximumIteration_; }

void InverseFORM::setVariableConvergence(Scalar variableConvergence)  { variableConvergence_ = variableConvergence; }
Scalar InverseFORM::getVariableConvergence() const { return variableConvergence_; }

void InverseFORM::setBetaConvergence(Scalar betaConvergence) { betaConvergence_ = betaConvergence; }
Scalar InverseFORM::getBetaConvergence() const { return betaConvergence_; }

void InverseFORM::setLimitStateConvergence(Scalar limitStateConvergence) { limitStateConvergence_ = limitStateConvergence; }
Scalar InverseFORM::getLimitStateConvergence() const { return limitStateConvergence_; }


/* Method save() stores the object through the StorageManager */
void InverseFORM::save(Advocate & adv) const
{
  PersistentObject::save(adv);
  adv.saveAttribute("event_", event_);
  adv.saveAttribute("parameterName_", parameterName_);
  adv.saveAttribute("physicalStartingPoint_", physicalStartingPoint_);
  adv.saveAttribute("fixedStep_", fixedStep_);
  adv.saveAttribute("fixedStepValue_", fixedStepValue_);
  adv.saveAttribute("variableStepMaxIterations_", variableStepMaxIterations_);
  adv.saveAttribute("maximumIteration_", maximumIteration_);
  adv.saveAttribute("variableConvergence_", variableConvergence_);
  adv.saveAttribute("betaConvergence_", betaConvergence_);
  adv.saveAttribute("limitStateConvergence_", limitStateConvergence_);
  adv.saveAttribute("result_", result_);
  
}

/* Method load() reloads the object from the StorageManager */
void InverseFORM::load(Advocate & adv)
{
  PersistentObject::load(adv);
  adv.loadAttribute("event_", event_);
  adv.loadAttribute("parameterName_", parameterName_);
  adv.loadAttribute("physicalStartingPoint_", physicalStartingPoint_);
  adv.loadAttribute("fixedStep_", fixedStep_);
  adv.loadAttribute("fixedStepValue_", fixedStepValue_);
  adv.loadAttribute("variableStepMaxIterations_", variableStepMaxIterations_);
  adv.loadAttribute("maximumIteration_", maximumIteration_);
  adv.loadAttribute("variableConvergence_", variableConvergence_);
  adv.loadAttribute("betaConvergence_", betaConvergence_);
  adv.loadAttribute("limitStateConvergence_", limitStateConvergence_);
  adv.loadAttribute("result_", result_);
}

}
