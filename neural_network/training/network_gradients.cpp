/*
Created by Abelentsev German
21.11.2026
*/

#include "neural_network/neural_network.hpp"

#include <utility>

#include "neural_network/instruments/activations.hpp"
#include "neural_network/training/loss.hpp"

NetworkGradients NeuralNetwork::Backward(const Matrix& input,
                                         const Matrix& targets,
                                         const Matrix& mask) const {
  const Matrix h1 = LeakyReLU(layer1_.Forward(input));
  const Matrix h2 = LeakyReLU(layer2_.Forward(h1));
  const Matrix logits = output_layer_.Forward(h2);

  const Matrix d_logits = MaskedBCEWithLogitsGradient(logits, targets, mask);
  DenseGradients g3 = output_layer_.Backward(h2, d_logits);

  const Matrix dz2 = LeakyReLUBackward(h2, g3.d_input);
  DenseGradients g2 = layer2_.Backward(h1, dz2);

  const Matrix dz1 = LeakyReLUBackward(h1, g2.d_input);
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
