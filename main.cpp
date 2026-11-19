#include <iostream>

#include "neural_network/instruments/activations.hpp"
#include "neural_network/dense_layer.hpp"
#include "neural_network/instruments/matrix.hpp"

int main() {
  Matrix w({{1, 2, 3}, {-4, -5, -6}});
  Matrix b({{1}, {2}});
  Matrix i({{.5}, {.8}, {0.0}});
  DenseLayer a(w, b);
  Matrix q = a.Forward(i);
  std::cout << q << '\n';
  q = ReLU(q);
  std::cout << q;
}
