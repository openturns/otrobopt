#include <iostream>

#include "otrobopt/OTRobOpt.hxx"
#include "openturns/Normal.hxx"
#include "openturns/LHSExperiment.hxx"

using namespace OT;
using namespace OTROBOPT;

int main(int argc, char **argv)
{
  Normal thetaDist(2.0, 0.1);
  Description input(2);
  input[0] = "x1";
  input[1] = "p1";
  NumericalMathFunction f(input, Description(1, "y1"), Description(1, "x1*p1"));
  NumericalMathFunction parametric(f, Indices(1 , 1), NumericalPoint(1, 1.0));

  NumericalPoint x(1, 1.0);

  Collection <MeasureEvaluation> measures;
  measures.add(MeanMeasure(thetaDist, parametric));
  measures.add(VarianceMeasure(thetaDist, parametric));
  measures.add(WorstCaseMeasure(thetaDist, parametric));
  measures.add(WorstCaseMeasure(thetaDist, parametric, false));
  measures.add(JointChanceMeasure(thetaDist, parametric, 0.95));
  measures.add(IndividualChanceMeasure(thetaDist, parametric, NumericalPoint(1, 0.95)));
  measures.add(MeanStandardDeviationTradeoffMeasure(thetaDist, parametric, NumericalPoint(1, 0.4)));
  measures.add(QuantileMeasure(thetaDist, parametric, 0.8));

  AggregatedMeasure aggregated(measures);
  measures.add(aggregated);

  const UnsignedInteger size = measures.getSize();
  for(UnsignedInteger i = 0; i < size; ++ i)
  {
    MeasureEvaluation measure(measures[i]);
    std::cout << measure << "(continuous)"<<measure(x)<< std::endl;
    const UnsignedInteger N = 1000;
    LHSExperiment experiment(thetaDist, N);
    MeasureFactory factory(measure, experiment);
    MeasureEvaluation discretizedMeasure(factory.build());
    std::cout << discretizedMeasure << "(discretized)"<<discretizedMeasure(x)<< std::endl;
  }
}

