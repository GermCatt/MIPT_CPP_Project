/*
Created by Abelentsev German
21.11.2026
*/

/*
Modified by Abelentsev German
22.11.2026
*/

#pragma once

#include <array>

#include "common/Config.hpp"

class Board;

class BoardInput {
 public:
  enum class Cell : unsigned char { Unknown = 0, Miss = 1, Hit = 2, Kill = 3 };
  static constexpr int cells = Config::board_size * Config::board_size;

  Cell GetCell(int x, int y) const;

 private:
  friend class Board;
  explicit BoardInput(const Board& board);

  std::array<Cell, cells> field_{};
};
