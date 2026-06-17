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
    // ---------------------------------------------------------------
    // 1) IndividualChanceMeasure with discrete distribution
    //    (exercises the discrete branch where weights index was wrong)
    // ---------------------------------------------------------------
    {
      Description input({"x", "theta"});
      SymbolicFunction f_base(input, Description({"x*theta"}));
      ParametricFunction f(f_base, Indices(1, 1), Point(1, 1.0));

      Point x(1, 2.0);

      // FiniteDiscreteDistribution discrete distribution with 5 points
      Sample discretePoints(5, 1);
      discretePoints(0, 0) = 0.0;
      discretePoints(1, 0) = 0.5;
      discretePoints(2, 0) = 1.0;
      discretePoints(3, 0) = 1.5;
      discretePoints(4, 0) = 2.0;
      Point probas(5, 0.2);
      FiniteDiscreteDistribution discreteDist(discretePoints, probas);

      IndividualChanceMeasure icm(f, discreteDist, Greater(), Point(1, 0.5));
      Point val(icm(x));
      fullprint << "IndividualChance (discrete)=" << val << std::endl;
      // theta values >= 0: all 5 points have f=2*theta >=0 => prob=1.0
      // Greater operator: result = P(f>=0) - alpha = 1.0 - 0.5 = 0.5
      assert_almost_equal(val, Point(1, 0.5), 1e-4, 1e-4);
    }

    // ---------------------------------------------------------------
    // 2) IndividualChanceMeasure with discrete distribution, multi-output
    // ---------------------------------------------------------------
    {
      Description input({"x", "theta"});
      SymbolicFunction f_base(input, Description({"x*theta", "x+theta"}));
      ParametricFunction f(f_base, Indices(1, 1), Point(1, 1.0));

      Point x(1, 1.0);
      Sample discretePoints(3, 1);
      discretePoints(0, 0) = -1.0;
      discretePoints(1, 0) = 0.0;
      discretePoints(2, 0) = 1.0;
      Point probas(3, 1.0 / 3.0);
      FiniteDiscreteDistribution discreteDist(discretePoints, probas);

      IndividualChanceMeasure icm(f, discreteDist, Greater(), Point(2, 0.5));
      Point val(icm(x));
      fullprint << "IndividualChance (discrete, multi-output)=" << val << std::endl;
      assert_almost_equal(val.getDimension(), 2);
    }

    // ---------------------------------------------------------------
    // 3) JointChanceMeasure with discrete distribution
    // ---------------------------------------------------------------
    {
      Description input({"x", "theta"});
      SymbolicFunction f_base(input, Description({"x*theta"}));
      ParametricFunction f(f_base, Indices(1, 1), Point(1, 1.0));

      Point x(1, 1.0);
      Sample discretePoints(3, 1);
      discretePoints(0, 0) = -1.0;
      discretePoints(1, 0) = 0.0;
      discretePoints(2, 0) = 1.0;
      Point probas(3, 1.0 / 3.0);
      FiniteDiscreteDistribution discreteDist(discretePoints, probas);

      JointChanceMeasure jcm(f, discreteDist, Greater(), 0.9);
      Point val(jcm(x));
      fullprint << "JointChance (discrete)=" << val << std::endl;
      assert_almost_equal(val.getDimension(), 1);
    }

    // ---------------------------------------------------------------
    // 4) QuantileMeasure with discrete distribution
    // ---------------------------------------------------------------
    {
      Description input({"x", "theta"});
      SymbolicFunction f_base(input, Description({"x*theta"}));
      ParametricFunction f(f_base, Indices(1, 1), Point(1, 1.0));

      Point x(1, 1.0);
      Sample discretePoints(3, 1);
      discretePoints(0, 0) = 0.0;
      discretePoints(1, 0) = 1.0;
      discretePoints(2, 0) = 2.0;
      Point probas(3, 1.0 / 3.0);
      FiniteDiscreteDistribution discreteDist(discretePoints, probas);

      QuantileMeasure qm(f, discreteDist, 0.5);
      Point val(qm(x));
      fullprint << "Quantile (discrete)=" << val << std::endl;
      assert_almost_equal(val.getDimension(), 1);
    }

    // ---------------------------------------------------------------
    // 5) MeanStandardDeviationTradeoffMeasure with discrete distribution
    // ---------------------------------------------------------------
    {
      Description input({"x", "theta"});
      SymbolicFunction f_base(input, Description({"x*theta"}));
      ParametricFunction f(f_base, Indices(1, 1), Point(1, 1.0));

      Point x(1, 1.0);
      Sample discretePoints(3, 1);
      discretePoints(0, 0) = 0.0;
      discretePoints(1, 0) = 1.0;
      discretePoints(2, 0) = 2.0;
      Point probas(3, 1.0 / 3.0);
      FiniteDiscreteDistribution discreteDist(discretePoints, probas);

      MeanStandardDeviationTradeoffMeasure mstd(f, discreteDist, Point(1, 0.5));
      Point val(mstd(x));
      fullprint << "MeanStdTradeoff (discrete)=" << val << std::endl;
      assert_almost_equal(val.getDimension(), 1);
    }

    // ---------------------------------------------------------------
    // 6) VarianceMeasure with discrete distribution
    // ---------------------------------------------------------------
    {
      Description input({"x", "theta"});
      SymbolicFunction f_base(input, Description({"x*theta"}));
      ParametricFunction f(f_base, Indices(1, 1), Point(1, 1.0));

      Point x(1, 1.0);
      Sample discretePoints(3, 1);
      discretePoints(0, 0) = 0.0;
      discretePoints(1, 0) = 1.0;
      discretePoints(2, 0) = 2.0;
      Point probas(3, 1.0 / 3.0);
      FiniteDiscreteDistribution discreteDist(discretePoints, probas);

      VarianceMeasure vm(f, discreteDist);
      Point val(vm(x));
      fullprint << "Variance (discrete)=" << val << std::endl;
      assert_almost_equal(val.getDimension(), 1);
    }

    // ---------------------------------------------------------------
    // 7) AggregatedMeasure with discrete distributions
    // ---------------------------------------------------------------
    {
      Description input({"x", "theta"});
      SymbolicFunction f_base(input, Description({"x*theta"}));
      ParametricFunction f(f_base, Indices(1, 1), Point(1, 1.0));

      Point x(1, 1.0);
      Sample discretePoints(3, 1);
      discretePoints(0, 0) = 0.0;
      discretePoints(1, 0) = 1.0;
      discretePoints(2, 0) = 2.0;
      Point probas(3, 1.0 / 3.0);
      FiniteDiscreteDistribution discreteDist(discretePoints, probas);

      Collection<MeasureEvaluation> measures;
      measures.add(MeanMeasure(f, discreteDist));
      measures.add(VarianceMeasure(f, discreteDist));

      AggregatedMeasure agg(measures);
      Point val(agg(x));
      fullprint << "Aggregated (discrete)=" << val << std::endl;
      assert_almost_equal(val.getDimension(), 2);
    }

    // ---------------------------------------------------------------
    // 8) MeasureEvaluation copy-on-write behavior
    // ---------------------------------------------------------------
    {
      Description input({"x", "theta"});
      SymbolicFunction f_base(input, Description({"x*theta"}));
      ParametricFunction f(f_base, Indices(1, 1), Point(1, 1.0));
      Normal thetaDist(2.0, 0.1);

      MeasureEvaluation m1(MeanMeasure(f, thetaDist));
      MeasureEvaluation m2(m1);
      // Both share the same implementation; m1 and m2 evaluate identically
      Point x(1, 1.0);
      assert_almost_equal(m1(x), m2(x));
    }
  }
  catch (TestFailed & ex)
  {
    std::cerr << ex << std::endl;
    return ExitCode::Error;
  }
  return ExitCode::Success;
}
