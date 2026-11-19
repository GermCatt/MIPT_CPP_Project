#include "neural_network/neural_network.hpp"

#include <utility>

#include "neural_network/dense_layer.hpp"
#include "neural_network/instruments/activations.hpp"
#include "neural_network/instruments/loss.hpp"

namespace {
Matrix ReLUBackward(const Matrix& activated, const Matrix& d_output) {
  Matrix d_input = d_output;
  for (int i = 0; i < activated.GetRows(); ++i) {
    if (activated(i, 0) <= 0.0) {
      d_input(i, 0) = 0.0;
    }
  }
  return d_input;
}
}  // namespace

NeuralNetwork::NeuralNetwork(const DenseLayer& layer1, const DenseLayer& layer2,
                             const DenseLayer& output_layer)
    : layer1_(layer1), layer2_(layer2), output_layer_(output_layer) {
  if (not(layer1_.GetWeights().GetRows() == layer2_.GetWeights().GetCols() &&
          layer2_.GetWeights().GetRows() ==
              output_layer_.GetWeights().GetCols())) {
    throw IncorrectDenseLayerError(
        "Dense layers do not correspond to one another");
  }
}

NeuralNetwork::NeuralNetwork(const Matrix& w1, const Matrix& b1,
                             const Matrix& w2, const Matrix& b2,
                             const Matrix& w3, const Matrix& b3)
    : layer1_(w1, b1), layer2_(w2, b2), output_layer_(w3, b3) {
  if (not(layer1_.GetWeights().GetRows() == layer2_.GetWeights().GetCols() &&
          layer2_.GetWeights().GetRows() ==
              output_layer_.GetWeights().GetCols())) {
    throw IncorrectDenseLayerError(
        "Dense layers do not correspond to one another");
  }
}

Matrix NeuralNetwork::ForwardLogits(const Matrix& input) const {
  Matrix h1 = ReLU(layer1_.Forward(input));
  Matrix h2 = ReLU(layer2_.Forward(h1));
  Matrix logits = output_layer_.Forward(h2);
  return logits;
}

Matrix NeuralNetwork::Predict(const Matrix& input) const {
  return Sigmoid(ForwardLogits(input));
}

NetworkGradients NeuralNetwork::Backward(const Matrix& input,
                                         const Matrix& targets,
                                         const Matrix& mask) const {
  const Matrix h1 = ReLU(layer1_.Forward(input));
  const Matrix h2 = ReLU(layer2_.Forward(h1));
  const Matrix logits = output_layer_.Forward(h2);

  const Matrix d_logits = MaskedBCEWithLogitsGradient(logits, targets, mask);
  DenseGradients g3 = output_layer_.Backward(h2, d_logits);

  const Matrix dz2 = ReLUBackward(h2, g3.d_input);
  DenseGradients g2 = layer2_.Backward(h1, dz2);

  const Matrix dz1 = ReLUBackward(h1, g2.d_input);
  DenseGradients g1 = layer1_.Backward(input, dz1);

  return {
      .layer1 = std::move(g1),
      .layer2 = std::move(g2),
      .output = std::move(g3),
  };
}

void NeuralNetwork::ApplyGradients(const NetworkGradients& gradients,
                                   double learning_rate) {
  layer1_.ApplyGradients(gradients.layer1, learning_rate);
  layer2_.ApplyGradients(gradients.layer2, learning_rate);
  output_layer_.ApplyGradients(gradients.output, learning_rate);
}