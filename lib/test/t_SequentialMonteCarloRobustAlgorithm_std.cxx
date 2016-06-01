#include <iostream>

#include "otrobopt/OTRobOpt.hxx"
#include "openturns/NLopt.hxx"
#include "openturns/Normal.hxx"
#include "openturns/Less.hxx"

using namespace OT;
using namespace OTROBOPT;

int main(int argc, char **argv)
{
  //Log.Show(Log.Info);

  Description input(2);
  input[0] = "x1";
  input[1] = "x2";

  // This is calligraphic J, the non-robust objective function
  NumericalMathFunction calJ(input, Description(1, "15.0 * (x1^2 + x2^2) - 100.0 * exp(-5. * ((x1 + 1.6)^2+(x2 + 1.6)^2))"));

  // This is calligraphic G, the non-robust inequality constraints function
  Description formula;
  formula.add("(x1 - 0.5)^2 + x2^2 - 4.0");
  formula.add("(x1 + 0.5)^2 + x2^2 - 4.0");
  NumericalMathFunction calG(input, formula);

  // This is the perturbation function
  input.add("xi1");
  input.add("xi2");
  formula = Description();
  formula.add("x1 + xi1");
  formula.add("x2 + xi2");
  NumericalMathFunction noise(input, formula);

  Indices paramSet;
  paramSet.add(2);
  paramSet.add(3);

  // This is capital J: J(x,xi) = calJ(x+xi), the parametric objective function
  NumericalMathFunction JFull(calJ, noise);
  NumericalMathFunction J(JFull, paramSet, NumericalPoint(2, 0.0));

  // This is g, the parametric constraints
  NumericalMathFunction gFull(calG, noise);
  NumericalMathFunction g(gFull, paramSet, NumericalPoint(2, 0.0));


  Interval bounds(NumericalPoint(2, -3.0), NumericalPoint(2, 3.0));
  NLopt solver("LD_SLSQP");
  solver.setMaximumIterationNumber(100);

  NumericalPoint sigma;
  sigma.add(0.1);
  sigma.add(0.2);
  sigma.add(0.3);
  sigma.add(0.4);
  sigma.add(0.5);

  for(UnsignedInteger i = 0; i < sigma.getSize(); ++ i)
  {
    Normal distributionXi(NumericalPoint(2, 0.0), NumericalPoint(2, sigma[i]), IdentityMatrix(2));
    MeanMeasure robustnessMeasure(J, distributionXi);
    JointChanceMeasure reliabilityMeasure(g, distributionXi, Less(), 0.9);
    RobustOptimizationProblem problem;
    problem.setRobustnessMeasure(robustnessMeasure);
    problem.setReliabilityMeasure(reliabilityMeasure);
    problem.setBounds(bounds);
    SequentialMonteCarloRobustAlgorithm algo(problem, solver);
    algo.setMaximumIterationNumber(11);
    algo.setMaximumAbsoluteError(1e-6);
    algo.setInitialSamplingSize(2); // size of initial xsi discretization, x2 at every iteration
    algo.setInitialSearch(1000); // number of multi-start tries, uniform law using bounds
    algo.run();
    OptimizationResult result(algo.getResult());
    std::cout << "x*=" << result.getOptimalPoint() << "J(x*)=" << result.getOptimalValue() << "iteration=" << result.getIterationNumber() << std::endl;
  }
}
