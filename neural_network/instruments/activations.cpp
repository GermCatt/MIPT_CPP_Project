/*
Created by Abelentsev German
19.11.2026
*/

#include "neural_network/instruments/activations.hpp"

#include <algorithm>
#include <cmath>

Matrix ReLU(const Matrix& input) {
  Matrix result(input.GetRows(), input.GetCols());
  for (int i = 0; i < input.GetRows(); ++i) {
    for (int j = 0; j < input.GetCols(); ++j) {
      result(i, j) = input(i, j) >= 0.0 ? input(i, j) : 0.0;
    }
  }
  return result;
}

Matrix Sigmoid(const Matrix& input) {
  Matrix result(input.GetRows(), input.GetCols());
  for (int i = 0; i < input.GetRows(); ++i) {
    for (int j = 0; j < input.GetCols(); ++j) {
      result(i, j) = 1 / (1 + std::exp(-1 * input(i, j)));
    }
  }
  return result;
}