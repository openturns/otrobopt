#include "openturns/OT.hxx"
#include "openturns/OTtestcode.hxx"
#include "otrobopt/OTRobOpt.hxx"

using namespace OT;
using namespace OT::Test;
using namespace OTROBOPT;

int main()
{
  TESTPREAMBLE;
  OStream fullprint(std::cout);

  try
  {
    Normal thetaDist(2.0, 0.1);
    SymbolicFunction f_base(Description({"x", "theta"}), Description({"x*theta"}));
    ParametricFunction f(f_base, Indices(1, 1), Point(1, 1.0));

    Point x(1, 1.0);

    // Test MeasureFactory build single measure
    MeasureEvaluation mean(MeanMeasure(f, thetaDist));
    LHSExperiment lhs(200);
    MeasureFactory factory(lhs);
    MeasureEvaluation discretized(factory.build(mean));
    fullprint << "discretized mean=" << discretized(x) << std::endl;

    // Test buildCollection
    Collection<MeasureEvaluation> measures;
    measures.add(MeanMeasure(f, thetaDist));
    measures.add(VarianceMeasure(f, thetaDist));

    MeasureFactory::MeasureEvaluationCollection coll(factory.buildCollection(measures));
    fullprint << "collection size=" << coll.getSize() << std::endl;
    fullprint << "coll[0]=" << coll[0](x) << std::endl;
    fullprint << "coll[1]=" << coll[1](x) << std::endl;
    assert_almost_equal(coll.getSize(), 2);

    // All items in a collection share the same distribution
    Distribution dist0(coll[0].getDistribution());
    Distribution dist1(coll[1].getDistribution());
    assert_equal(dist0.getImplementation().get(), dist1.getImplementation().get());
  }
  catch (TestFailed & ex)
  {
    std::cerr << ex << std::endl;
    return ExitCode::Error;
  }
  return ExitCode::Success;
}
