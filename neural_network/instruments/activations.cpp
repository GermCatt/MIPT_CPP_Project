/*
Created by Abelentsev German
19.11.2026
*/

/*
Modified by Abelentsev German
21.11.2026
*/

#include "neural_network/instruments/activations.hpp"

#include <cmath>
#include <utility>

namespace {
constexpr double negative_slope = 0.01;

template <class Function>
Matrix ApplyActivation(Matrix input, Function activate) {
  for (int row = 0; row < input.GetRows(); ++row) {
    for (int col = 0; col < input.GetCols(); ++col) {
      input(row, col) = activate(input(row, col));
    }
  }
  return input;
}

double LeakyValue(double x) {
  return x < 0.0 ? x * negative_slope : x;
}
}

Matrix ReLU(const Matrix& input) {
  return ApplyActivation(input, [](double x) { return x >= 0.0 ? x : 0.0; });
}

Matrix LeakyReLU(const Matrix& input) {
  return ApplyActivation(input, LeakyValue);
}

Matrix LeakyReLU(Matrix&& input) {
  return ApplyActivation(std::move(input), LeakyValue);
}

Matrix LeakyReLUBackward(const Matrix& activated, const Matrix& d_output) {
  if (activated.GetRows() != d_output.GetRows() ||
      activated.GetCols() != d_output.GetCols()) {
    throw InvalidMatrixSizeError("LeakyReLU gradient shape mismatch");
  }
  Matrix d_input = d_output;
  for (int i = 0; i < activated.GetRows(); ++i) {
    for (int j = 0; j < activated.GetCols(); ++j) {
      if (activated(i, j) <= 0.0) d_input(i, j) *= negative_slope;
    }
  }
  return d_input;
}

Matrix Sigmoid(const Matrix& input) {
  return ApplyActivation(input, [](double x) { return 1 / (1 + std::exp(-x)); });
}
