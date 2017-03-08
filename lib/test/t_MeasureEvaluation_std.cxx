#include <iostream>

#include "otrobopt/OTRobOpt.hxx"
#include "openturns/Normal.hxx"
#include "openturns/ComposedDistribution.hxx"
#include "openturns/LHSExperiment.hxx"
#include "openturns/GaussProductExperiment.hxx"
#include "openturns/GreaterOrEqual.hxx"
#include "openturns/Uniform.hxx"
#include "openturns/SymbolicFunction.hxx"
#include "openturns/ParametricFunction.hxx"

using namespace OT;
using namespace OTROBOPT;

int main(int argc, char **argv)
{
  // First test: theta of dimension 1
  {
    Normal thetaDist(2.0, 0.1);
    Description input(2);
    input[0] = "x";
    input[1] = "theta";
    SymbolicFunction f_base(input, Description(1, "x*theta"));
    ParametricFunction f(f_base, Indices(1, 1), NumericalPoint(1, 1.0));

    NumericalPoint x(1, 1.0);

    Collection <MeasureEvaluation> measures;
    measures.add(MeanMeasure(f, thetaDist));
    measures.add(VarianceMeasure(f, thetaDist));
    measures.add(WorstCaseMeasure(f, Uniform(-1.0, 4.0)));
    measures.add(WorstCaseMeasure(f, Uniform(-1.0, 4.0), false));
    measures.add(JointChanceMeasure(f, Normal(1.0, 1.0), GreaterOrEqual(), 0.95));
    measures.add(IndividualChanceMeasure(f, Normal(1.0, 1.0), GreaterOrEqual(), NumericalPoint(1, 0.95)));
    measures.add(MeanStandardDeviationTradeoffMeasure(f, thetaDist, NumericalPoint(1, 0.8)));
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
    Normal thetaDist(NumericalPoint(2, 2.0), NumericalPoint(2, 0.1), IdentityMatrix(2));
    Description input(3);
    input[0] = "x";
    input[1] = "theta0";
    input[2] = "theta1";
    SymbolicFunction f_base(input, Description(1, "x*theta0+theta1"));
    Indices indices(2);
    indices[0] = 1;
    indices[1] = 2;
    ParametricFunction f(f_base, indices, thetaDist.getMean());

    NumericalPoint x(1, 1.0);

    Collection <MeasureEvaluation> measures;
    measures.add(MeanMeasure(f, thetaDist));
    measures.add(VarianceMeasure(f, thetaDist));
    measures.add(WorstCaseMeasure(f, ComposedDistribution(Collection<Distribution>(2, Uniform(-1.0, 4.0)))));
    measures.add(WorstCaseMeasure(f, ComposedDistribution(Collection<Distribution>(2, Uniform(-1.0, 4.0))), false));
    measures.add(JointChanceMeasure(f, thetaDist, GreaterOrEqual(), 0.5));
    measures.add(IndividualChanceMeasure(f, thetaDist, GreaterOrEqual(), NumericalPoint(1, 0.5)));
    measures.add(MeanStandardDeviationTradeoffMeasure(f, thetaDist, NumericalPoint(1, 0.8)));
    measures.add(QuantileMeasure(f, thetaDist, 0.5));

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

