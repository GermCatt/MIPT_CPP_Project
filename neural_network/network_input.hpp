/*
Created by Abelentsev German
22.11.2026
*/

#pragma once

#include <array>

#include "Game/board_input.hpp"
#include "neural_network/instruments/matrix.hpp"

using ShotOrder = std::array<int, BoardInput::cells>;

namespace NetworkInput {
inline constexpr int features_per_cell = 4;
Matrix Encode(const BoardInput& board);
}  // namespace NetworkInput
