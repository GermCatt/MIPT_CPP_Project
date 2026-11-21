/*
Created by Abelentsev German
19.11.2026
*/

#pragma once
#include "neural_network/dense_layer.hpp"

struct NetworkGradients {
  DenseGradients layer1;
  DenseGradients layer2;
  DenseGradients output;
};

class NeuralNetwork final {
 private:
  DenseLayer layer1_;
  DenseLayer layer2_;
  DenseLayer output_layer_;

 public:
  NeuralNetwork(const DenseLayer& layer1, const DenseLayer& layer2,
                const DenseLayer& output_layer);
  NeuralNetwork(const Matrix& w1, const Matrix& b1, const Matrix& w2,
                const Matrix& b2, const Matrix& w3, const Matrix& b3);

  Matrix ForwardLogits(const Matrix& input) const;
  Matrix Predict(const Matrix& input) const;

  const DenseLayer& GetLayer1() const { return layer1_; }
  const DenseLayer& GetLayer2() const { return layer2_; }
  const DenseLayer& GetOutputLayer() const { return output_layer_; }

  NetworkGradients Backward(const Matrix& input, const Matrix& targets,
                            const Matrix& mask) const;
  void ApplyGradients(const NetworkGradients& gradients, double learning_rate);
};
