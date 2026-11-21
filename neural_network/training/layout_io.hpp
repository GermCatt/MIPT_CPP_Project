/*
Created by Abelentsev German
21.11.2026
*/

#pragma once

#include <stdexcept>
#include <string>

#include "Game/board.hpp"
#include "common/Config.hpp"
#include "common/validate.hpp"

inline Field DecodeLayout(const std::string& line) {
  constexpr int side = Config::board_size, cells = side * side;
  if (line.size() != cells || line.find_first_not_of("01") != std::string::npos)
    throw std::invalid_argument(
        "Expected one binary character per cell, row-major");
  Field field(side, std::vector<CellType>(side, CellType::Empty));
  for (int y = 0; y < side; ++y) {
    for (int x = 0; x < side; ++x) {
      if (line[y * side + x] == '0' || field[y][x] != CellType::Empty) continue;
      const bool horizontal = x + 1 < side && line[y * side + x + 1] == '1';
      const int dx = horizontal, dy = !horizontal;
      int length = 1;
      while (x + length * dx < side && y + length * dy < side &&
             line[(y + length * dy) * side + x + length * dx] == '1')
        ++length;
      if (length > 4) throw std::invalid_argument("Ship exceeds four decks");
      for (int k = 0; k < length; ++k)
        field[y + k * dy][x + k * dx] = static_cast<CellType>(length);
    }
  }
  Validate::ValidatePlacement(field);
  return field;
}
