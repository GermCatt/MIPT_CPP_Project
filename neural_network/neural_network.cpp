/*
Created by Abelentsev German
19.11.2026
*/

/*
Modified by Abelentsev German
21.11.2026
*/

#include "neural_network/neural_network.hpp"

#include "neural_network/dense_layer.hpp"
#include "neural_network/instruments/activations.hpp"

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
  Matrix h1 = LeakyReLU(layer1_.Forward(input));
  Matrix h2 = LeakyReLU(layer2_.Forward(h1));
  Matrix logits = output_layer_.Forward(h2);
  return logits;
}

Matrix NeuralNetwork::Predict(const Matrix& input) const {
  return Sigmoid(ForwardLogits(input));
}
