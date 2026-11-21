/*
Created by Abelentsev German
21.11.2026
*/

#pragma once

#include "neural_network/board_input.hpp"

struct TrainingExample {
  Matrix input;
  Matrix targets;
  Matrix mask;
};

TrainingExample MakeTrainingExample(const Board& board);
