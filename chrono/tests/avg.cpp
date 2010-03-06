#include <iostream>
#include <cmath>
#include "chrono.h"

void test_with_ctor()
{
  double msecs = 0.0;
  int c = 0;
  for (; c<10000; ++c) {
    Chrono chrono;
    std::sin(0.0);
    msecs += chrono.elapsed();
  }
  std::cout << "sin(0) in " << (msecs/c) << " milliseconds (average from " << c << " runs)\n";
}

void test_with_reset()
{
  Chrono chrono;
  double msecs = 0.0;
  int c = 0;
  for (; c<10000; ++c) {
    chrono.reset();
    std::sin(0.0);
    msecs += chrono.elapsed();
  }
  std::cout << "sin(0) in " << (msecs/c) << " milliseconds (average from " << c << " runs)\n";
}

int main()
{
  test_with_ctor();
  test_with_reset();
  return 0;
}

