#include "openturns/OT.hxx"
#include "openturns/OTtestcode.hxx"
#include "otrobopt/OTRobOpt.hxx"

using namespace OT;
using namespace OT::Test;
using namespace OTROBOPT;


int main()
{
  TESTPREAMBLE;
  Log::Show(Log::INFO);
  OStream fullprint(std::cout);

  try
  {
    // beam model
    const UnsignedInteger dim = 4;

    const SymbolicFunction function(Description({"E", "F", "L", "b", "h"}), Description({"F*L^3/(48.*E*b*h^3/12.)"}));
    ParametricFunction parametric(function, Indices({2}), Point({5.0}));

    JointDistribution::DistributionCollection coll;
    coll.add(LogNormalMuSigmaOverMu(30000., 0.12, 0.).getDistribution());//E
    coll.add(LogNormalMuSigmaOverMu(0.1, 0.2, 0.).getDistribution());//F
    coll.add(LogNormalMuSigmaOverMu(0.2, 0.05, 0.).getDistribution());//b
    coll.add(LogNormalMuSigmaOverMu(0.4, 0.05, 0.).getDistribution());//h

    const JointDistribution myDistribution(coll);

    Point median(dim);
    for(UnsignedInteger i = 0; i < dim; ++ i)
      median[i] = myDistribution.getMarginal(i).computeQuantile(0.5)[0];

    /* We create a 'usual' RandomVector from the Distribution */
    const RandomVector vect(myDistribution);

    /* We create a composite random vector */
    CompositeRandomVector output(parametric, vect);

    /* We create an Event from this RandomVector */
    ThresholdEvent myEvent(output, Greater(), 0.02);

    fullprint << "event=" << myEvent << std::endl;

    // create the algorithm
    InverseFORM algo(myEvent, "L", median);

    // print result
    algo.run();
    const InverseFORMResult result(algo.getResult());
    fullprint << "Inverse FORM result=" << result << std::endl;
    assert_almost_equal(result.getParameter(), Point({5.44343}));

    // FORM must yield the same probability on the limit state with parameter set to the optimum
    parametric.setParameter(result.getParameter());
    output = CompositeRandomVector(parametric, vect);
    myEvent = ThresholdEvent(output, Greater(), 0.02);

    Cobyla cobyla;
    cobyla.setMaximumIterationNumber(100);
    cobyla.setMaximumAbsoluteError(1.0e-10);
    cobyla.setMaximumRelativeError(1.0e-10);
    cobyla.setMaximumResidualError(1.0e-10);
    cobyla.setMaximumConstraintError(1.0e-10);

    FORM myFORM(cobyla, myEvent, median);
    myFORM.run();
    const FORMResult resultFORM(myFORM.getResult());
    fullprint << "FORM result="<< resultFORM << std::endl;
    assert_almost_equal(resultFORM.getHasoferReliabilityIndex(), algo.getTargetBeta());
  }
  catch (TestFailed & ex)
  {
    std::cerr << ex << std::endl;
    return ExitCode::Error;
  }
  return ExitCode::Success;
}

