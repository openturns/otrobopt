#include <iostream>

#include "otrobopt/OTRobOpt.hxx"
#include "openturns/OT.hxx"


using namespace OT;
using namespace OTROBOPT;

int main()
{
  // First test: theta of dimension 1
  {
    Normal thetaDist(2.0, 0.1);
    Description input(2);
    input[0] = "x";
    input[1] = "theta";
    SymbolicFunction f_base(input, Description(1, "x*theta"));
    ParametricFunction f(f_base, Indices(1 , 1), Point(1, 1.0));

    Point x(1, 1.0);

    Collection <MeasureEvaluation> measures;
    measures.add(MeanMeasure(f, thetaDist));
    measures.add(VarianceMeasure(f, thetaDist));
    measures.add(WorstCaseMeasure(f, Uniform(-1.0, 4.0)));
    measures.add(WorstCaseMeasure(f, Uniform(-1.0, 4.0), false));
    measures.add(JointChanceMeasure(f, Normal(1.0, 1.0), GreaterOrEqual(), 0.95));
    measures.add(IndividualChanceMeasure(f, Normal(1.0, 1.0), GreaterOrEqual(), Point(1, 0.95)));
    measures.add(MeanStandardDeviationTradeoffMeasure(f, thetaDist, Point(1, 0.8)));
    measures.add(QuantileMeasure(f, thetaDist, 0.99));

    AggregatedMeasure aggregated(measures);
    measures.add(aggregated);

    const UnsignedInteger size = measures.getSize();
    for(UnsignedInteger i = 0; i < size; ++ i)
    {
      MeasureEvaluation measure(measures[i]);
      std::cout << measure << "(continuous)" << measure(x) << std::endl;
      {
        const UnsignedInteger N = 1000;
        LHSExperiment experiment(N);
        MeasureFactory factory(experiment);
        MeasureEvaluation discretizedMeasure(factory.build(measure));
        std::cout << discretizedMeasure << "(discretized LHS)" << discretizedMeasure(x) << std::endl;
      }
      {
        const UnsignedInteger N = 4;
        GaussProductExperiment experiment(Indices(1, N));
        MeasureFactory factory(experiment);
        MeasureEvaluation discretizedMeasure(factory.build(measure));
        std::cout << discretizedMeasure << "(discretized Gauss)" << discretizedMeasure(x) << std::endl;
      }
    }
  }
  // Second test: theta of dimension 2
  {
    Normal thetaDist(Point(2, 2.0), Point(2, 0.1), IdentityMatrix(2));
    Description input(3);
    input[0] = "x";
    input[1] = "theta0";
    input[2] = "theta1";
    SymbolicFunction f_base(input, Description(1, "x*theta0+theta1"));
    Indices indices(2);
    indices[0] = 1;
    indices[1] = 2;
    ParametricFunction f(f_base, indices, thetaDist.getMean());

    Point x(1, 1.0);

    Collection <MeasureEvaluation> measures;
    measures.add(MeanMeasure(f, thetaDist));
    measures.add(VarianceMeasure(f, thetaDist));
    measures.add(WorstCaseMeasure(f, JointDistribution(Collection<Distribution>(2, Uniform(-1.0, 4.0)))));
    measures.add(WorstCaseMeasure(f, JointDistribution(Collection<Distribution>(2, Uniform(-1.0, 4.0))), false));
    measures.add(JointChanceMeasure(f, thetaDist, GreaterOrEqual(), 0.5));
    measures.add(IndividualChanceMeasure(f, thetaDist, GreaterOrEqual(), Point(1, 0.5)));
    measures.add(MeanStandardDeviationTradeoffMeasure(f, thetaDist, Point(1, 0.8)));
    // FIXME: QuantileMeasure got slow with OT 1.2rc1, in the Brent step
    // measures.add(QuantileMeasure(f, thetaDist, 0.5));

    AggregatedMeasure aggregated(measures);
    measures.add(aggregated);

    const UnsignedInteger size = measures.getSize();
    for(UnsignedInteger i = 0; i < size; ++ i)
    {
      MeasureEvaluation measure(measures[i]);
      std::cout << measure << "(continuous)" << measure(x) << std::endl;
      {
        const UnsignedInteger N = 1000;
        LHSExperiment experiment(N);
        MeasureFactory factory(experiment);
        MeasureEvaluation discretizedMeasure(factory.build(measure));
        std::cout << discretizedMeasure << "(discretized LHS)" << discretizedMeasure(x) << std::endl;
      }
      {
        const UnsignedInteger N = 4;
        GaussProductExperiment experiment(measure.getDistribution(), Indices(2, N));
        MeasureFactory factory(experiment);
        MeasureEvaluation discretizedMeasure(factory.build(measure));
        std::cout << discretizedMeasure << "(discretized Gauss)" << discretizedMeasure(x) << std::endl;
      }
    }
  }
}

