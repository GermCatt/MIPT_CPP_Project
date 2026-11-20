/*
Created by Abelentsev German
19.11.2026
*/

#include <filesystem>
#include <iostream>

#include "common/weights.hpp"
#include "neural_network/instruments/loss.hpp"
#include "neural_network/neural_network.hpp"

int main() try {
  const std::filesystem::path path = "models/weights.txt";
  const Matrix w1({{0.5, -0.2}, {0.2, 0.4}, {-0.1, 0.3}});
  const Matrix w2({{0.4, 0.3, -0.1}, {0.2, 0.5, 0.1}});
  const Matrix w3({{0.2, 0.3}, {-0.2, 0.1}, {0.4, 0.2}});
  const bool resume = std::filesystem::exists(path);
  NeuralNetwork network =
      resume
          ? LoadWeights(path)
          : NeuralNetwork(w1, Matrix(3, 1), w2, Matrix(2, 1), w3, Matrix(3, 1));
  if (resume) std::cout << "Loaded " << path << '\n';

  const Matrix input({{1.0}, {0.0}});
  const Matrix targets({{1.0}, {0.0}, {1.0}});
  const Matrix mask({{1.0}, {1.0}, {1.0}});
  constexpr double learning_rate = 0.05;
  constexpr int steps = 1000;

  for (int step = 0; step < steps; ++step) {
    if (step % 100 == 0) {
      const double loss =
          MaskedBCEWithLogits(network.ForwardLogits(input), targets, mask);
      std::cout << "step " << step << ", loss " << loss << '\n';
    }
    const NetworkGradients gradients = network.Backward(input, targets, mask);
    network.ApplyGradients(gradients, learning_rate);
  }

  std::cout << "final loss: "
            << MaskedBCEWithLogits(network.ForwardLogits(input), targets, mask)
            << "\npredictions:\n"
            << network.Predict(input) << '\n';
  SaveWeights(network, path);
  std::cout << "Saved " << path << '\n';
} catch (const std::exception& error) {
  std::cerr << "Training failed: " << error.what() << '\n';
  return 1;
}