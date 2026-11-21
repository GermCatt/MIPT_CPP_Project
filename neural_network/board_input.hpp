/*
Created by Abelentsev German
21.11.2026
*/

#pragma once

#include <array>

#include "Game/board.hpp"
#include "common/Config.hpp"
#include "neural_network/instruments/matrix.hpp"

namespace BoardInput {
inline constexpr int cells = Config::board_size * Config::board_size;
inline constexpr int features_per_cell = 4;
int VisibleChannel(CellType cell);
}  // namespace BoardInput

using ShotOrder = std::array<int, BoardInput::cells>;
Matrix MakeVisibleInput(const Board& board);
