/*
Created by Abelentsev German
21.11.2026
*/

#include "board_input.hpp"

int BoardInput::VisibleChannel(CellType cell) {
  switch (cell) {
    case CellType::Miss:
      return 1;
    case CellType::Hit:
      return 2;
    case CellType::Kill:
      return 3;
    default:
      return 0;
  }
}

Matrix MakeVisibleInput(const Board& board) {
  constexpr int size = Config::board_size;
  Matrix input(BoardInput::features_per_cell * BoardInput::cells, 1);
  for (int i = 0; i < BoardInput::cells; ++i) {
    const int channel =
        BoardInput::VisibleChannel(board.GetCell(i % size, i / size));
    input(i * BoardInput::features_per_cell + channel, 0) = 1;
  }
  return input;
}
