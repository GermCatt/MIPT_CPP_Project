/*
Created by Abelentsev German
22.11.2026
*/

#pragma once

#include "Game/board.hpp"
#include "common/Config.hpp"
#include "neural_network/instruments/matrix.hpp"

struct TrainingExample {
  static constexpr int cells = Config::board_size * Config::board_size;
  static constexpr int features_per_cell = 4;

  Matrix input;
  Matrix targets;
  Matrix mask;
};

TrainingExample MakeTrainingExample(const Board& board);
