#include <iostream>
#include <cmath>
#include "chrono.h"

int main()
{
  double msecs;
  {
    Chrono chrono;
    std::sin(0.0);
    msecs = chrono.elapsed();
  }
  std::cout << "sin(0) in " << msecs << " milliseconds\n";
  return 0;
}

