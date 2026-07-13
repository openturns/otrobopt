#include <iostream>
#include <openturns/OT.hxx>
#include "otrobopt/RobustOptimizationAlgorithm.hxx"
#include "otrobopt/MeanMeasure.hxx"
#include "otrobopt/JointChanceMeasure.hxx"

using namespace OT;
using namespace OTROBOPT;

int main()
{
  // Test 1: default constructor
  {
    RobustOptimizationAlgorithm algo;
    std::cout << "default: " << algo.__repr__() << std::endl;
  }

  // Test 2: parameter constructor and getters
  {
    Normal thetaDist(2.0, 0.1);
    Description input(2);
    input[0] = "x";
    input[1] = "theta";
    SymbolicFunction f_base(input, Description(1, "x*theta"));
    ParametricFunction f(f_base, Indices(1, 1), Point(1, 1.0));

    MeasureEvaluation robustness(MeanMeasure(f, thetaDist));
    MeasureEvaluation reliability(JointChanceMeasure(f, thetaDist, Less(), 0.5));
    RobustOptimizationProblem problem(robustness, reliability);

    OptimizationAlgorithm solver{Cobyla()};
    RobustOptimizationAlgorithm algo(problem, solver);
    std::cout << "parameter: " << algo.__repr__() << std::endl;

    RobustOptimizationProblem gotProblem = algo.getRobustProblem();
    std::cout << "got problem: " << gotProblem.__repr__() << std::endl;

    OptimizationAlgorithm gotSolver = algo.getOptimizationAlgorithm();
    std::cout << "got solver: " << gotSolver.__repr__() << std::endl;
  }

  // Test 3: set/get robust problem and solver
  {
    Normal thetaDist(2.0, 0.1);
    Description input(2);
    input[0] = "x";
    input[1] = "theta";
    SymbolicFunction f_base(input, Description(1, "x*theta"));
    ParametricFunction f(f_base, Indices(1, 1), Point(1, 1.0));

    MeasureEvaluation robustness(MeanMeasure(f, thetaDist));
    MeasureEvaluation reliability(JointChanceMeasure(f, thetaDist, Less(), 0.5));
    RobustOptimizationProblem problem(robustness, reliability);

    RobustOptimizationAlgorithm algo;
    OptimizationAlgorithm solver{Cobyla()};
    algo.setRobustProblem(problem);
    algo.setOptimizationAlgorithm(solver);
    std::cout << "set/get: " << algo.__repr__() << std::endl;
  }

  // Test 4: clone
  {
    Normal thetaDist(2.0, 0.1);
    Description input(2);
    input[0] = "x";
    input[1] = "theta";
    SymbolicFunction f_base(input, Description(1, "x*theta"));
    ParametricFunction f(f_base, Indices(1, 1), Point(1, 1.0));

    MeasureEvaluation robustness(MeanMeasure(f, thetaDist));
    MeasureEvaluation reliability(JointChanceMeasure(f, thetaDist, Less(), 0.5));
    RobustOptimizationProblem problem(robustness, reliability);

    RobustOptimizationAlgorithm algo(problem, OptimizationAlgorithm{Cobyla()});
    RobustOptimizationAlgorithm * cloned = algo.clone();
    std::cout << "clone: " << cloned->__repr__() << std::endl;
    delete cloned;
  }

  return 0;
}
