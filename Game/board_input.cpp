/*
Created by Abelentsev German
21.11.2026
*/

/*
Modified by Abelentsev German
22.11.2026
*/

#include "Game/board_input.hpp"

#include "Game/board.hpp"
#include "common/errors.hpp"

BoardInput::BoardInput(const Board& board) {
  for (int y = 0; y < Config::board_size; ++y) {
    for (int x = 0; x < Config::board_size; ++x) {
      Cell& visible = field_[y * Config::board_size + x];
      switch (board.GetCell(x, y)) {
        case CellType::Miss:
          visible = Cell::Miss;
          break;
        case CellType::Hit:
          visible = Cell::Hit;
          break;
        case CellType::Kill:
          visible = Cell::Kill;
          break;
        default:
          visible = Cell::Unknown;
          break;
      }
    }
  }
}

BoardInput::Cell BoardInput::GetCell(int x, int y) const {
  if (x < 0 || y < 0 || x >= Config::board_size || y >= Config::board_size)
    throw InvalidArgumentError("Координаты клетки за пределами поля");
  return field_[y * Config::board_size + x];
}
