/*
Created by Abelentsev German
22.11.2026
*/

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string_view>

#include "common/weights.hpp"
#include "neural_network/placement_generator.hpp"
#include "neural_network/training_example.hpp"

namespace {
constexpr int size = Config::board_size;
constexpr int cells = TrainingExample::cells;
using ShotOrder = std::array<int, cells>;

template <typename Score>
int ChooseShot(const Matrix& mask, const ShotOrder& order, Score score) {
  int best = -1;
  double best_score = 0;
  for (int cell : order) {
    if (mask(cell, 0) == 0) continue;
    const double value = score(cell);
    if (best == -1 || value > best_score) {
      best = cell;
      best_score = value;
    }
  }
  if (best == -1) throw InvalidValueError("No available shot");
  return best;
}

int HuntScore(const Matrix& input, int cell) {
  auto hit = [&](int x, int y) {
    return x >= 0 && x < size && y >= 0 && y < size &&
           input(TrainingExample::features_per_cell * (y * size + x) + 2, 0) ==
               1;
  };
  const int x = cell % size, y = cell / size;
  int score = 0;
  for (int sign : {-1, 1}) {
    if (hit(x + sign, y)) {
      if (hit(x + 2 * sign, y)) return 2;
      score = 1;
    }
    if (hit(x, y + sign)) {
      if (hit(x, y + 2 * sign)) return 2;
      score = 1;
    }
  }
  return score;
}

template <typename NextShot>
int Play(const Field& placement, NextShot next_shot) {
  Board board(std::make_unique<Field>(placement));
  int ships_left = Config::single_deck + Config::double_deck +
                   Config::three_deck + Config::four_deck;
  for (int turn = 0; turn < cells; ++turn) {
    const int cell = next_shot(board, turn);
    const int x = cell % size, y = cell / size;
    board.Shoot(x, y);
    if (board.GetCell(x, y) == CellType::Kill && --ships_left == 0)
      return turn + 1;
  }
  throw InvalidValueError("Fleet still afloat after all shots");
}
}  // namespace

int main(int argc, char* argv[]) try {
  if (argc > 2) throw InvalidArgumentError("Usage: evaluate.exe [seed]");
  unsigned seed = 42;
  if (argc == 2) {
    const std::string_view arg(argv[1]);
    const char* end = arg.data() + arg.size();
    const auto parsed = std::from_chars(arg.data(), end, seed);
    if (parsed.ec != std::errc{} || parsed.ptr != end)
      throw InvalidArgumentError("Invalid seed");
  }
  const auto network = LoadWeights("models/battleship_weights.txt");
  if (network.GetLayer1().GetWeights().GetCols() !=
          cells * TrainingExample::features_per_cell ||
      network.GetOutputLayer().GetWeights().GetRows() != cells) {
    throw InvalidValueError("Weights do not match the board encoding");
  }

  constexpr int games = 500;
  std::mt19937 layouts(seed), shots(seed ^ 0x9e3779b9u);
  int net_total = 0, random_total = 0, hunt_total = 0;
  for (int game = 0; game < games; ++game) {
    const auto placement = GeneratePlacement(layouts);
    ShotOrder order;
    std::iota(order.begin(), order.end(), 0);
    std::shuffle(order.begin(), order.end(), shots);
    net_total += Play(*placement, [&](const Board& board, int) {
      const auto example = MakeTrainingExample(board);
      const auto scores = network.ForwardLogits(example.input);
      return ChooseShot(example.mask, order, [&](int cell) {
        const double value = scores(cell, 0);
        if (!std::isfinite(value))
          throw InvalidValueError("Non-finite network score");
        return value;
      });
    });
    random_total +=
        Play(*placement, [&](const Board&, int turn) { return order[turn]; });
    hunt_total += Play(*placement, [&](const Board& board, int) {
      const auto example = MakeTrainingExample(board);
      return ChooseShot(example.mask, order, [&](int cell) {
        return HuntScore(example.input, cell);
      });
    });
  }
  std::cout << "Boards: " << games << ", seed: " << seed
            << "\nMean shots to sink the fleet\n"
            << std::fixed << std::setprecision(3)
            << "Network: " << static_cast<double>(net_total) / games << '\n'
            << "Random:  " << static_cast<double>(random_total) / games << '\n'
            << "Hunt:    " << static_cast<double>(hunt_total) / games << '\n';
} catch (const std::exception& error) {
  std::cerr << "Evaluation failed: " << error.what() << '\n';
  return 1;
}
