/*
Created by Abelentsev German
21.11.2026
*/

/*
Modified by Abelentsev German
22.11.2026
*/

#pragma once

#include "Game/board.hpp"
#include "neural_network/network_input.hpp"

struct TrainingExample {
  Matrix input;
  Matrix targets;
  Matrix mask;
};

TrainingExample MakeTrainingExample(const Board& board);
