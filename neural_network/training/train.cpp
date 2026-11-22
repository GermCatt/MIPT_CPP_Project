/*
Created by Abelentsev German
19.11.2026
*/

/*
Modified by Abelentsev German
22.11.2026
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
#include <utility>
#include <vector>

#include "common/weights.hpp"
#include "neural_network/placement_generator.hpp"
#include "neural_network/training/loss.hpp"
#include "neural_network/training/shot_policy.hpp"
#include "neural_network/training/training_example.hpp"

namespace {
constexpr int cells = BoardInput::cells;
constexpr int inputs = cells * NetworkInput::features_per_cell;
constexpr int hidden = 64;
constexpr double learning_rate = 0.05;

int ParsePositiveInt(std::string_view argument, const char* error) {
  const char* end = argument.data() + argument.size();
  int games = 0;
  const auto result = std::from_chars(argument.data(), end, games);
  if (result.ec != std::errc{} || result.ptr != end || games <= 0) {
    throw InvalidArgumentError(error);
  }
  return games;
}

Matrix RandomWeights(int outputs, int inputs_per_neuron, std::mt19937& rng) {
  Matrix weights(outputs, inputs_per_neuron);
  std::normal_distribution<double> sample(0.0,
                                          std::sqrt(2.0 / inputs_per_neuron));
  for (int row = 0; row < outputs; ++row) {
    for (int col = 0; col < inputs_per_neuron; ++col) {
      weights(row, col) = sample(rng);
    }
  }
  return weights;
}

NeuralNetwork CreateNetwork(std::mt19937& rng, int first_hidden = hidden) {
  const Matrix w1 = RandomWeights(first_hidden, inputs, rng);
  const Matrix w2 = RandomWeights(hidden, first_hidden, rng);
  const Matrix w3 = RandomWeights(cells, hidden, rng);
  return NeuralNetwork(w1, Matrix(first_hidden, 1), w2, Matrix(hidden, 1), w3,
                       Matrix(cells, 1));
}

template <typename Visit>
void GenerateGame(std::mt19937& rng, Visit visit,
                  const NeuralNetwork* player = nullptr, bool masked = false) {
  Board board(GeneratePlacement(rng));
  std::array<int, cells> shots;
  std::iota(shots.begin(), shots.end(), 0);
  std::shuffle(shots.begin(), shots.end(), rng);
  int ships_left = Config::single_deck + Config::double_deck +
                   Config::three_deck + Config::four_deck;
  for (int turn = 0; turn < cells; ++turn) {
    auto example = MakeTrainingExample(board);
    int shot = shots[turn];
    if (player) {
      if (masked) MaskKnownWater(example);
      const Matrix scores = player->ForwardLogits(example.input);
      shot = ChooseNetworkShot(example, scores, shots, masked);
    }
    visit(std::move(example));
    const int x = shot % Config::board_size;
    const int y = shot / Config::board_size;
    board.Shoot(x, y);
    if (board.GetCell(x, y) == CellType::Kill && --ships_left == 0) break;
  }
}

void TrainOn(NeuralNetwork& model, const TrainingExample& e) {
  model.ApplyGradients(model.Backward(e.input, e.targets, e.mask),
                       learning_rate);
}

void FineTune(int games, const std::filesystem::path& source,
              const std::filesystem::path& destination, unsigned seed,
              bool masked) {
  if (std::filesystem::exists(destination) &&
      std::filesystem::equivalent(source, destination)) {
    throw InvalidArgumentError("Fine-tuning needs a separate output file");
  }
  auto network = LoadWeights(source);
  if (network.GetLayer1().GetWeights().GetCols() != inputs ||
      network.GetOutputLayer().GetWeights().GetRows() != cells) {
    throw InvalidValueError("Weights do not match the training example");
  }
  std::mt19937 game_rng(seed), shuffle_rng(842377);
  std::cout << "Fine-tuning from " << source << ", board seed " << seed << '\n';
  constexpr int block_size = 64;
  constexpr double fine_tune_rate = 0.005;
  std::vector<TrainingExample> positions;
  positions.reserve(block_size * cells);
  for (int done = 0; done < games;) {
    const int block = std::min(games - done, block_size);
    positions.clear();
    for (int i = 0; i < block; ++i) {
      GenerateGame(
          game_rng,
          [&](TrainingExample e) { positions.push_back(std::move(e)); },
          &network, masked);
    }
    std::shuffle(positions.begin(), positions.end(), shuffle_rng);
    for (const auto& e : positions) {
      network.ApplyGradients(network.Backward(e.input, e.targets, e.mask),
                             fine_tune_rate);
    }
    done += block;
    std::cout << "Fine-tuned " << done << '/' << games
              << ", positions in block " << positions.size() << std::endl;
  }
  SaveWeights(network, destination);
  std::cout << "Saved " << destination << '\n';
}

void TrainShuffled(NeuralNetwork& shuffled, int games,
                   NeuralNetwork* ordered = nullptr, bool checkpoints = false) {
  std::mt19937 game_rng(781), shuffle_rng(2179);
  constexpr int block_size = 64;
  std::vector<TrainingExample> positions;
  positions.reserve(block_size * cells);
  for (int done = 0; done < games;) {
    const int block = std::min(games - done, block_size);
    positions.clear();
    for (int game = 0; game < block; ++game) {
      GenerateGame(game_rng, [&](TrainingExample e) {
        positions.push_back(std::move(e));
      });
    }
    if (ordered) {
      for (const auto& e : positions) TrainOn(*ordered, e);
    }
    std::shuffle(positions.begin(), positions.end(), shuffle_rng);
    for (const auto& e : positions) TrainOn(shuffled, e);
    done += block;
    std::cout << (ordered ? "Paired games " : "Games ") << done << '/' << games
              << ", positions per model in block " << positions.size()
              << std::endl;
    if (checkpoints && (done == block_size || done % 4096 == 0) &&
        done < games) {
      const auto dir =
          std::filesystem::path("checkpoints") / std::to_string(done);
      if (ordered) SaveWeights(*ordered, dir / ordered_weights_path);
      SaveWeights(shuffled, dir / shuffled_weights_path);
      std::cout << "Checkpoint " << done << std::endl;
    }
  }
}
}  // namespace

int main(int argc, char* argv[]) try {
  const bool compare =
      argc >= 2 && std::string_view(argv[1]) == "--compare-shuffle";
  const bool train_width =
      argc >= 2 && std::string_view(argv[1]) == "--train-width";
  const bool masked =
      argc >= 2 && std::string_view(argv[1]) == "--fine-tune-masked";
  const bool fine_tune =
      masked || (argc >= 2 && std::string_view(argv[1]) == "--fine-tune");
  const bool checkpoints =
      compare && argc == 4 && std::string_view(argv[3]) == "--checkpoints";
  if ((compare && argc != 3 && !checkpoints) ||
      (fine_tune && argc != 5 && argc != 6) || (train_width && argc != 5) ||
      (!compare && !fine_tune && !train_width && argc > 2)) {
    throw InvalidArgumentError(
        "Usage: app.exe [games] | --compare-shuffle games [--checkpoints] | "
        "--train-width games first_hidden output | --fine-tune[-masked] games "
        "source output [seed]");
  }
  const int games =
      argc == 1
          ? 16
          : ParsePositiveInt(argv[compare || fine_tune || train_width ? 2 : 1],
                             "Game count must be a positive integer");
  if (Config::board_size != 10 || Config::single_deck != 4 ||
      Config::double_deck != 3 || Config::three_deck != 2 ||
      Config::four_deck != 1) {
    throw InvalidValueError(
        "Training requires a 10x10 board and the standard fleet");
  }

  if (fine_tune) {
    unsigned seed;
    if (argc == 6) {
      const std::string_view arg = argv[5];
      const auto parsed =
          std::from_chars(arg.data(), arg.data() + arg.size(), seed);
      if (parsed.ec != std::errc{} || parsed.ptr != arg.data() + arg.size()) {
        throw InvalidArgumentError("Invalid fine-tuning seed");
      }
    } else {
      seed = std::random_device{}();
    }
    FineTune(games, argv[3], argv[4], seed, masked);
    return 0;
  }
  if (train_width || compare) {
    std::mt19937 init_rng(318);
    if (train_width) {
      const int first_hidden = ParsePositiveInt(
          argv[3], "First hidden width must be a positive integer");
      auto network = CreateNetwork(init_rng, first_hidden);
      TrainShuffled(network, games);
      SaveWeights(network, argv[4]);
    } else {
      auto ordered = CreateNetwork(init_rng);
      auto shuffled = ordered;
      TrainShuffled(shuffled, games, &ordered, checkpoints);
      SaveWeights(ordered, ordered_weights_path);
      SaveWeights(shuffled, shuffled_weights_path);
    }
    return 0;
  }
  std::mt19937 rng(std::random_device{}());
  const std::filesystem::path path = battleship_weights_path;
  const bool resume = std::filesystem::exists(path);
  NeuralNetwork network = resume ? LoadWeights(path) : CreateNetwork(rng);
  if (network.GetLayer1().GetWeights().GetCols() != inputs ||
      network.GetLayer1().GetWeights().GetRows() != hidden ||
      network.GetLayer2().GetWeights().GetRows() != hidden ||
      network.GetOutputLayer().GetWeights().GetRows() != cells) {
    throw InvalidValueError("Unexpected Battleship network dimensions");
  }
  std::cout << (resume ? "Resuming" : "Starting") << " training (" << inputs
            << " -> " << hidden << " -> " << hidden << " -> " << cells << ")\n";

  for (int game = 0; game < games; ++game) {
    int positions = 0;
    double loss_sum = 0.0;
    GenerateGame(rng, [&](const TrainingExample& example) {
      const Matrix logits = network.ForwardLogits(example.input);
      const double loss =
          MaskedBCEWithLogits(logits, example.targets, example.mask);
      if (!std::isfinite(loss))
        throw InvalidValueError("Non-finite training loss");
      loss_sum += loss;
      ++positions;

      TrainOn(network, example);
    });
    std::cout << "Game " << (game + 1) << '/' << games << ", positions "
              << positions << ", mean loss " << loss_sum / positions << '\n';
  }

  SaveWeights(network, path);
  std::cout << "Saved " << path << '\n';
  return 0;
} catch (const std::exception& error) {
  std::cerr << "Training failed: " << error.what() << '\n';
  return 1;
}
