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
    // sphere under pressure
    const UnsignedInteger dim = 3;
    const SymbolicFunction function(Description({"R", "e", "mulog_e", "p"}), Description({"700.0-p*R/(2.*e)"}));

    const Scalar L0 = -4.715;
    ParametricFunction parametric(function, Indices({2}), Point({L0}));

    Dirac mulog_eDist(L0);
    mulog_eDist.setDescription(Description(1, "mulog_e"));
    JointDistribution::DistributionCollection eColl; eColl.add(mulog_eDist); eColl.add(Dirac(0.1));  eColl.add(Dirac(0.));
    JointDistribution eParams(eColl);

    JointDistribution::DistributionCollection coll;
    coll.add(Beta(0.117284, 0.117284, 2.9, 3.1));//R
#if OPENTURNS_VERSION >= 102400
    DeconditionedDistribution eDist(LogNormal(L0, 0.1, 0.), eParams);
#else
    ConditionalDistribution eDist(LogNormal(L0, 0.1, 0.), eParams);
#endif
    coll.add(eDist);//e
    coll.add(WeibullMin(3.16471, 9.21097, 0.0));//p
    JointDistribution myDistribution(coll);

    Point median(dim);
    for(UnsignedInteger i = 0; i < dim; ++ i)
      median[i] = myDistribution.getMarginal(i).computeQuantile(0.5)[0];

    /* We create a 'usual' RandomVector from the Distribution */
    RandomVector vect(myDistribution);

    /* We create a composite random vector */
    CompositeRandomVector output(parametric, vect);

    /* We create an Event from this RandomVector */
    ThresholdEvent event(output, Less(), 0.);

    fullprint << "event=" << event << std::endl;

    // create the algorithm
    InverseFORM algo(event, "mulog_e", median);

    // print result
    algo.run();
    const InverseFORMResult result(algo.getResult());
    fullprint << "Inverse FORM result=" << result << std::endl;
    assert_almost_equal(result.getParameter(), Point({-4.69056}), 0.0, 0.03);

    // FORM must yield the same probability on the limit state with parameter set to the optimum
    eColl[0] = Dirac(result.getParameter()[0]);
    eParams = JointDistribution(eColl);
#if OPENTURNS_VERSION >= 102400
    eDist = DeconditionedDistribution(LogNormal(result.getParameter()[0], 0.1, 0.0), eParams);
#else
    eDist = ConditionalDistribution(LogNormal(result.getParameter()[0], 0.1, 0.0), eParams);
#endif
    coll[1] = eDist;
    myDistribution = JointDistribution(coll);
    vect = RandomVector(myDistribution);
    parametric.setParameter(result.getParameter());
    output = CompositeRandomVector(parametric, vect);
    event = ThresholdEvent(output, Less(), 0.);

    Cobyla cobyla;
    cobyla.setMaximumIterationNumber(100);
    cobyla.setMaximumAbsoluteError(1.0e-10);
    cobyla.setMaximumRelativeError(1.0e-10);
    cobyla.setMaximumResidualError(1.0e-10);
    cobyla.setMaximumConstraintError(1.0e-10);
    
    FORM form(cobyla, event, median);
    form.run();
    const FORMResult resultFORM(form.getResult());
    fullprint << "result FORM="<<resultFORM << std::endl;
    assert_almost_equal(resultFORM.getHasoferReliabilityIndex(), algo.getTargetBeta(), 0.0, 0.1);
  }
  catch (const TestFailed & ex)
  {
    std::cerr << ex << std::endl;
    return ExitCode::Error;
  }
  return ExitCode::Success;
}

