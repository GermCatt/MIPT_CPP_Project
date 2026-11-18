#include <iostream>

#include "neural_network/instruments/matrix.hpp"

int main() {
  Matrix matrix(2, 2);
  std::cout << matrix << '\n';
  return 0;
}
