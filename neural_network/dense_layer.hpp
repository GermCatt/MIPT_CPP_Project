/*
Created by Abelentsev German
19.11.2026
*/

#pragma once

#include "instruments\matrix.hpp"

struct DenseGradients {
  Matrix d_weights;
  Matrix d_bias;
  Matrix d_input;
};

class DenseLayer {
 private:
  Matrix weights_;
  Matrix bias_;

 public:
  DenseGradients Backward(const Matrix& input, const Matrix& d_output) const;
  DenseLayer(const Matrix& weights, const Matrix& bias);
  Matrix Forward(const Matrix& input) const;
  const Matrix& GetBias() const;
  const Matrix& GetWeights() const;
  void ApplyGradients(const DenseGradients& gradients, double learning_rate);
};