/*
Created by Abelentsev German
22.11.2026
*/

#include "neural_network/network_input.hpp"

Matrix NetworkInput::Encode(const BoardInput& board) {
  Matrix input(features_per_cell * BoardInput::cells, 1);
  for (int i = 0; i < BoardInput::cells; ++i) {
    const auto cell =
        board.GetCell(i % Config::board_size, i / Config::board_size);
    input(i * features_per_cell + static_cast<int>(cell), 0) = 1;
  }
  return input;
}
