/*
Created by Abelentsev German
21.11.2026
*/

#include "Game/computer_opponent.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <stdexcept>

#include "common/weights.hpp"
#include "neural_network/hard_network.hpp"

ComputerOpponent::ComputerOpponent(Difficulty difficulty, unsigned seed,
                                   const std::filesystem::path& models) {
  if (Config::board_size != 10 || Config::single_deck != 4 ||
      Config::double_deck != 3 || Config::three_deck != 2 ||
      Config::four_deck != 1) {
    throw std::invalid_argument(
        "These models require the standard 10x10 fleet");
  }
  std::iota(order_.begin(), order_.end(), 0);
  std::mt19937 rng(seed);
  std::shuffle(order_.begin(), order_.end(), rng);

  if (difficulty == Difficulty::Easy) {
    auto network = LoadWeights(models / "easy.txt");
    if (network.GetLayer1().GetWeights().GetCols() !=
            BoardInput::features_per_cell * BoardInput::cells ||
        network.GetOutputLayer().GetWeights().GetRows() != BoardInput::cells)
      throw std::runtime_error("Easy weights do not match the board");
    choose_ = [network = std::move(network)](const Matrix& input,
                                             const ShotOrder& order) {
      const auto scores = network.ForwardLogits(input);
      int best = -1;
      for (int cell : order) {
        if (input(BoardInput::features_per_cell * cell, 0) == 0) continue;
        if (!std::isfinite(scores(cell, 0)))
          throw std::runtime_error("Non-finite easy network score");
        if (best < 0 || scores(cell, 0) > scores(best, 0)) best = cell;
      }
      if (best < 0) throw std::runtime_error("No remaining shot");
      return best;
    };
    return;
  }
  choose_ = [policy = HardNetwork::Load(models / "hard.txt")](
                const Matrix& input, const ShotOrder& order) {
    return policy.Choose(input, order);
  };
}

std::pair<int, int> ComputerOpponent::ChooseShot(const Board& board) const {
  const int cell = choose_(MakeVisibleInput(board), order_);
  return {cell % Config::board_size, cell / Config::board_size};
}
