#include <iostream>
#include <openturns/OT.hxx>
#include "otrobopt/OTRobOpt.hxx"

using namespace OT;
using namespace OTROBOPT;

int main()
{
  Normal thetaDist(2.0, 0.1);
  Description input(2);
  input[0] = "x";
  input[1] = "theta";
  SymbolicFunction f_base(input, Description(1, "x*theta"));
  ParametricFunction f(f_base, Indices(1, 1), Point(1, 1.0));

  // Test 1: construct with robustness + reliability measures
  {
    MeasureEvaluation robustness(MeanMeasure(f, thetaDist));
    MeasureEvaluation reliability(JointChanceMeasure(f, thetaDist, Less(), 0.5));
    RobustOptimizationProblem problem(robustness, reliability);
    std::cout << "test1: " << problem.__repr__() << std::endl;
    std::cout << "test1 hasRobust: " << problem.hasRobustnessMeasure() << std::endl;
    std::cout << "test1 hasReliab: " << problem.hasReliabilityMeasure() << std::endl;
  }

  // Test 2: construct with objective + reliability measure
  {
    SymbolicFunction objective("x", "x");
    MeasureEvaluation reliability(JointChanceMeasure(f, thetaDist, Less(), 0.5));
    RobustOptimizationProblem problem(objective, reliability);
    std::cout << "test2: " << problem.__repr__() << std::endl;
    std::cout << "test2 hasRobust: " << problem.hasRobustnessMeasure() << std::endl;
    std::cout << "test2 hasReliab: " << problem.hasReliabilityMeasure() << std::endl;
  }

  // Test 3: construct with robustness measure + inequality constraint
  {
    SymbolicFunction constraint("x", "x");
    MeasureEvaluation robustness(MeanMeasure(f, thetaDist));
    RobustOptimizationProblem problem(robustness, constraint);
    std::cout << "test3: " << problem.__repr__() << std::endl;
    std::cout << "test3 hasRobust: " << problem.hasRobustnessMeasure() << std::endl;
    std::cout << "test3 hasReliab: " << problem.hasReliabilityMeasure() << std::endl;
  }

  // Test 4: default constructor and set/get
  {
    MeasureEvaluation robustness(MeanMeasure(f, thetaDist));
    MeasureEvaluation reliability(JointChanceMeasure(f, thetaDist, Less(), 0.5));
    RobustOptimizationProblem problem;
    problem.setRobustnessMeasure(robustness);
    problem.setReliabilityMeasure(reliability);
    std::cout << "test4 getRobust: " << problem.getRobustnessMeasure().__repr__() << std::endl;
    std::cout << "test4 getReliab: " << problem.getReliabilityMeasure().__repr__() << std::endl;
    std::cout << "test4 dist: " << problem.getDistribution().__repr__() << std::endl;
  }

  // Test 5: clone
  {
    MeasureEvaluation robustness(MeanMeasure(f, thetaDist));
    MeasureEvaluation reliability(JointChanceMeasure(f, thetaDist, Less(), 0.5));
    RobustOptimizationProblem problem(robustness, reliability);
    RobustOptimizationProblem * cloned = problem.clone();
    std::cout << "test5 clone: " << cloned->__repr__() << std::endl;
    delete cloned;
  }

  // Test 6: default constructor
  {
    RobustOptimizationProblem problem;
    std::cout << "test6 hasRobust: " << problem.hasRobustnessMeasure() << std::endl;
    std::cout << "test6 hasReliab: " << problem.hasReliabilityMeasure() << std::endl;
  }

  return 0;
}
