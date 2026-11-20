/*
Created by Abelentsev German
19.11.2026
*/

#include "neural_network/dense_layer.hpp"

#include "common/errors.hpp"
#include "instruments\matrix.hpp"

DenseLayer::DenseLayer(const Matrix& weights, const Matrix& bias)
    : weights_(weights), bias_(bias) {
  if (not(weights_.GetRows() == bias.GetRows() && bias_.GetCols() == 1)) {
    throw InvalidBiasSizeError("Bias size is incorrect");
  }
}

Matrix DenseLayer::Forward(const Matrix& input) const {
  if (input.GetCols() != 1) {
    throw InvalidMatrixSizeError("Activations must have one row");
  }
  return weights_ * input + bias_;
}

const Matrix& DenseLayer::GetBias() const { return bias_; }

const Matrix& DenseLayer::GetWeights() const { return weights_; }

DenseGradients DenseLayer::Backward(const Matrix& input,
                                    const Matrix& d_output) const {
  if (input.GetCols() != 1 || input.GetRows() != weights_.GetCols()) {
    throw InvalidMatrixSizeError(
        "Invalid input shape for DenseLayer::Backward");
  }

  if (d_output.GetCols() != 1 || d_output.GetRows() != weights_.GetRows()) {
    throw InvalidMatrixSizeError("Invalid output gradient shape");
  }

  return {
      .d_weights = d_output * input.Transposed(),
      .d_bias = d_output,
      .d_input = weights_.Transposed() * d_output,
  };
}

void DenseLayer::ApplyGradients(const DenseGradients& gradients,
                                double learning_rate) {
  weights_ -= learning_rate * gradients.d_weights;
  bias_ -= learning_rate * gradients.d_bias;
}