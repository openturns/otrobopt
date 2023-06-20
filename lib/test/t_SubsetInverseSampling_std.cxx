#include <iostream>
#include <openturns/OT.hxx>
#include "otrobopt/SubsetInverseSampling.hxx"

using namespace OT;
using namespace OTROBOPT;

int main()
{
  Distribution myDistribution = Normal();
  RandomVector vect(myDistribution);
  
  SymbolicFunction limitState("X","2*X");
  CompositeRandomVector output(limitState, vect);

  ThresholdEvent myEvent(output, Less(), 0.0);
    
  Scalar finalProbability(0.0001);

  SubsetInverseSampling ssi(myEvent, finalProbability);
  std::cout << ssi << std::endl;
  ssi.run();
  std::cout << ssi.getResult() << std::endl;
  return 0;
}
