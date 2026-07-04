//                                               -*- C++ -*-
/**
 *  @brief Test for SORAAlgorithm
 *
 *  Copyright 2005-2024 Airbus-EDF-IMACS-Phimeca
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "otrobopt/OTRobOpt.hxx"
#include "openturns/OT.hxx"

using namespace OT;
using namespace OTROBOPT;

int main()
{
  // Analytical RBDO test case using SORA

  // Objective: f(d) = d0 + d1
  Description objInput(2);
  objInput[0] = "d0";
  objInput[1] = "d1";
  SymbolicFunction objective(objInput, Description(1, "d0 + d1"));

  // Limit state: g(d1, d2, x1, x2) = d1 + d2 - x1 - x2
  Description input(4);
  input[0] = "d1";
  input[1] = "d2";
  input[2] = "x1";
  input[3] = "x2";
  SymbolicFunction g(input, Description(1, "d1 + d2 - x1 - x2"));

  // Distribution of X (standard normal)
  Normal distX(Point(2, 0.0), Point(2, 1.0), IdentityMatrix(2));

  // No Z distribution
  Distribution distZ;

  // Target beta
  const Scalar targetBeta = 3.0;

  // Active indices
  Indices activeD;
  activeD.add(0);
  activeD.add(1);
  Indices activeP;

  // Bounds
  Interval bounds(Point(2, 0.0), Point(2, 10.0));

  // Build the RBDO problem
  RBDOProblem rbdoProblem(objective, g, distX, distZ, targetBeta, Less(),
                          activeD, activeP);
  rbdoProblem.setBounds(bounds);
  rbdoProblem.setDesignDimension(2);
  rbdoProblem.setParameterDimension(0);

  // Solver
  NLopt solver("LD_SLSQP");
  solver.setMaximumIterationNumber(100);

  // Run SORA
  SORAAlgorithm sora(rbdoProblem, solver);
  sora.setStartingPoint(Point(2, 5.0));
  sora.setMaximumSORAIteration(20);
  sora.setSORATolerance(1e-3);
  sora.run();

  const Point xStar(sora.getResult().getOptimalPoint());
  std::cout << "SORA x*=" << xStar << std::endl;

  if (xStar.getDimension() == 2)
  {
    std::cout << "d1+d2=" << xStar[0] + xStar[1] << std::endl;
  }

  return 0;
}
