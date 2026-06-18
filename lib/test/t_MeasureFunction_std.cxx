#include "openturns/OT.hxx"
#include "openturns/OTtestcode.hxx"
#include "otrobopt/OTRobOpt.hxx"
#include "otrobopt/MeasureFunction.hxx"

using namespace OT;
using namespace OT::Test;
using namespace OTROBOPT;

int main()
{
  TESTPREAMBLE;
  OStream fullprint(std::cout);

  try
  {
    // theta of dimension 1
    Normal thetaDist(2.0, 0.1);
    SymbolicFunction f_base(Description({"x", "theta"}), Description({"x*theta"}));
    ParametricFunction f(f_base, Indices(1, 1), Point(1, 1.0));

    MeasureEvaluation meanMeasure(MeanMeasure(f, thetaDist));

    // Test MeasureFunction wrapping
    MeasureFunction func(meanMeasure);
    fullprint << "MeasureFunction=" << func << std::endl;
    fullprint << "input dimension=" << func.getInputDimension() << std::endl;
    fullprint << "output dimension=" << func.getOutputDimension() << std::endl;

    // operator() on a single point
    Point x(1, 1.0);
    Point y(func(x));
    fullprint << "eval(1.0)=" << y << std::endl;
    assert_almost_equal(y, Point(1, 2.0), 1e-4, 1e-4);

    // operator() on a sample
    Sample inS(3, 1);
    inS(0, 0) = 0.5;
    inS(1, 0) = 1.0;
    inS(2, 0) = 2.0;
    Sample outS(func(inS));
    fullprint << "sample eval=" << outS << std::endl;
    assert_almost_equal(outS(0, 0), 1.0, 1e-4, 1e-4);
    assert_almost_equal(outS(1, 0), 2.0, 1e-4, 1e-4);
    assert_almost_equal(outS(2, 0), 4.0, 1e-4, 1e-4);

    // clone
    MeasureFunction cloned(func);
    Point y2(cloned(x));
    assert_almost_equal(y2, y);
  }
  catch (TestFailed & ex)
  {
    std::cerr << ex << std::endl;
    return ExitCode::Error;
  }
  return ExitCode::Success;
}
