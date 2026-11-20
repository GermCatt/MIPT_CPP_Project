/*
Created by Abelentsev German
19.11.2026
*/

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <random>
#include <string_view>

#include "common/weights.hpp"
#include "neural_network/instruments/loss.hpp"
#include "neural_network/placement_generator.hpp"
#include "neural_network/training_example.hpp"

namespace {
constexpr int cells = TrainingExample::cells;
constexpr int inputs = cells * TrainingExample::features_per_cell;
constexpr int hidden = 64;

int ParseGameCount(int argc, char* argv[]) {
  if (argc == 1) return 16;
  if (argc != 2) throw InvalidArgumentError("Usage: app.exe [games]");

  const std::string_view argument = argv[1];
  const char* end = argument.data() + argument.size();
  int games = 0;
  const auto result = std::from_chars(argument.data(), end, games);
  if (result.ec != std::errc{} || result.ptr != end || games <= 0) {
    throw InvalidArgumentError("Game count must be a positive integer");
  }
  return games;
}

Matrix RandomWeights(int outputs, int inputs_per_neuron, std::mt19937& rng) {
  Matrix weights(outputs, inputs_per_neuron);
  std::normal_distribution<double> sample(0.0, std::sqrt(2.0 / inputs_per_neuron));
  for (int row = 0; row < outputs; ++row) {
    for (int col = 0; col < inputs_per_neuron; ++col) {
      weights(row, col) = sample(rng);
    }
  }
  return weights;
}

NeuralNetwork CreateNetwork(std::mt19937& rng) {
  const Matrix w1 = RandomWeights(hidden, inputs, rng);
  const Matrix w2 = RandomWeights(hidden, hidden, rng);
  const Matrix w3 = RandomWeights(cells, hidden, rng);
  return NeuralNetwork(w1, Matrix(hidden, 1), w2, Matrix(hidden, 1), w3, Matrix(cells, 1));
}
}  // namespace

int main(int argc, char* argv[]) try {
  const int games = ParseGameCount(argc, argv);
  if (Config::board_size != 10 || Config::single_deck != 4 || Config::double_deck != 3 ||
      Config::three_deck != 2 || Config::four_deck != 1) {
    throw InvalidValueError("Training requires a 10x10 board and the standard fleet");
  }

  std::mt19937 rng(std::random_device{}());
  const std::filesystem::path path = "models/battleship_weights.txt";
  const bool resume = std::filesystem::exists(path);
  NeuralNetwork network = resume ? LoadWeights(path) : CreateNetwork(rng);
  if (network.GetLayer1().GetWeights().GetCols() != inputs ||
      network.GetLayer1().GetWeights().GetRows() != hidden ||
      network.GetLayer2().GetWeights().GetRows() != hidden ||
      network.GetOutputLayer().GetWeights().GetRows() != cells) {
    throw InvalidValueError("Unexpected Battleship network dimensions");
  }
  std::cout << (resume ? "Resuming" : "Starting") << " training ("
            << inputs << " -> " << hidden << " -> " << hidden << " -> " << cells << ")\n";

  constexpr double learning_rate = 0.05;
  for (int game = 0; game < games; ++game) {
    Board board(GeneratePlacement(rng));
    std::array<int, cells> shots;
    std::iota(shots.begin(), shots.end(), 0);
    std::shuffle(shots.begin(), shots.end(), rng);

    int ships_left = Config::single_deck + Config::double_deck +
                     Config::three_deck + Config::four_deck;
    int positions = 0;
    double loss_sum = 0.0;
    for (int shot : shots) {
      const TrainingExample example = MakeTrainingExample(board);
      const Matrix logits = network.ForwardLogits(example.input);
      const double loss = MaskedBCEWithLogits(logits, example.targets, example.mask);
      if (!std::isfinite(loss)) throw InvalidValueError("Non-finite training loss");
      loss_sum += loss;
      ++positions;

      const auto gradients = network.Backward(example.input, example.targets, example.mask);
      network.ApplyGradients(gradients, learning_rate);

      const int x = shot % Config::board_size;
      const int y = shot / Config::board_size;
      board.Shoot(x, y);
      if (board.GetCell(x, y) == CellType::Kill && --ships_left == 0) break;
    }
    std::cout << "Game " << (game + 1) << '/' << games << ", positions " << positions
              << ", mean loss " << loss_sum / positions << '\n';
  }

  SaveWeights(network, path);
  std::cout << "Saved " << path << '\n';
  return 0;
} catch (const std::exception& error) {
  std::cerr << "Training failed: " << error.what() << '\n';
  return 1;
}
