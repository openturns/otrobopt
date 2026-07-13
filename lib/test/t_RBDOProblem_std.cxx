#include <iostream>
#include <openturns/OT.hxx>
#include "otrobopt/OTRobOpt.hxx"

using namespace OT;
using namespace OTROBOPT;

int main()
{
  // Test 1: default constructor
  {
    RBDOProblem problem;
    std::cout << "default targetBeta: " << problem.getTargetBeta() << std::endl;
    std::cout << "default designDimension: " << problem.getDesignDimension() << std::endl;
    std::cout << "default parameterDimension: " << problem.getParameterDimension() << std::endl;
    std::cout << "default repr: " << problem.__repr__() << std::endl;
  }

  // Test 2: parameter constructor
  {
    // Simple objective: minimize d
    SymbolicFunction objective("d", "d");

    // Constraint: g(d, X) = d - X >= 0  (i.e. d >= X)
    // With X ~ Normal(0,1), P[d-X >= 0] >= target -> P[X <= d] >= target
    // We want reliability index beta = 1.0
    SymbolicFunction constraintFunc(Description{"d", "x0"}, Description(1, "d - x0"));

    Distribution distX(Normal(0.0, 1.0));
    Distribution distZ{Normal()};

    Scalar targetBeta = 1.0;
    Indices activeDesignIndices(1, 0);  // d is the first variable
    Indices activeParameterIndices;     // no parameters

    RBDOProblem problem(objective, constraintFunc, distX, distZ,
                        targetBeta, GreaterOrEqual(),
                        activeDesignIndices, activeParameterIndices);
    std::cout << "parameter constructor: " << problem.__repr__() << std::endl;

    std::cout << "hasRobustness: " << problem.hasRobustnessMeasure() << std::endl;
    std::cout << "hasReliability: " << problem.hasReliabilityMeasure() << std::endl;
    std::cout << "targetBeta: " << problem.getTargetBeta() << std::endl;
    std::cout << "designDimension: " << problem.getDesignDimension() << std::endl;
    std::cout << "parameterDimension: " << problem.getParameterDimension() << std::endl;
    std::cout << "hasZ: " << problem.hasDistributionZ() << std::endl;
  }

  // Test 3: setters and getters
  {
    RBDOProblem problem;

    SymbolicFunction constraintFunc(Description{"d", "x0"}, Description(1, "d - x0"));
    problem.setConstraintFunction(constraintFunc);
    std::cout << "constraint function: " << problem.getConstraintFunction().__repr__() << std::endl;

    Distribution distX(Normal(0.0, 1.0));
    problem.setDistributionX(distX);
    std::cout << "distX: " << problem.getDistributionX().__repr__() << std::endl;

    Distribution distZ(Uniform(0.0, 1.0));
    problem.setDistributionZ(distZ);
    std::cout << "distZ: " << problem.getDistributionZ().__repr__() << std::endl;
    std::cout << "hasZ after set: " << problem.hasDistributionZ() << std::endl;

    problem.setTargetBeta(2.0);
    std::cout << "targetBeta: " << problem.getTargetBeta() << std::endl;

    problem.setConstraintOperator(Less());
    std::cout << "constraint operator: " << problem.getConstraintOperator().__repr__() << std::endl;

    Indices designInd(1, 0);
    problem.setActiveDesignIndices(designInd);
    std::cout << "activeDesignIndices: " << problem.getActiveDesignIndices().__repr__() << std::endl;

    Indices paramInd(1, 1);
    problem.setActiveParameterIndices(paramInd);
    std::cout << "activeParameterIndices: " << problem.getActiveParameterIndices().__repr__() << std::endl;

    problem.setDesignDimension(2);
    std::cout << "designDimension: " << problem.getDesignDimension() << std::endl;

    problem.setParameterDimension(3);
    std::cout << "parameterDimension: " << problem.getParameterDimension() << std::endl;
  }

  // Test 4: clone
  {
    SymbolicFunction objective("d", "d");
    SymbolicFunction constraintFunc(Description{"d", "x0"}, Description(1, "d - x0"));
    Distribution distX(Normal(0.0, 1.0));
    Distribution distZ{Normal()};
    Indices activeDesignIndices(1, 0);

    RBDOProblem problem(objective, constraintFunc, distX, distZ, 1.0,
                        GreaterOrEqual(), activeDesignIndices, Indices());
    RBDOProblem * cloned = problem.clone();
    std::cout << "clone: " << cloned->__repr__() << std::endl;
    delete cloned;
  }

  return 0;
}
