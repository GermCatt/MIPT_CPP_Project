/*
Created by Abelentsev German
19.11.2026
*/

#include "placement_generator.hpp"

#include <algorithm>
#include <array>

#include "common/Config.hpp"
#include "common/errors.hpp"
#include "common/validate.hpp"

namespace {
constexpr int size = Config::board_size;
constexpr std::array ship_counts{0, Config::single_deck, Config::double_deck,
                                 Config::three_deck, Config::four_deck};

bool HasRoom(const Field& field, int x, int y, int width, int height) {
  const int last_row = std::min(size - 1, y + height);
  const int last_col = std::min(size - 1, x + width);
  for (int row = std::max(0, y - 1); row <= last_row; ++row) {
    for (int col = std::max(0, x - 1); col <= last_col; ++col) {
      if (field[row][col] != CellType::Empty) return false;
    }
  }
  return true;
}

bool PlaceShip(Field& field, int length, std::mt19937& rng) {
  constexpr int max_attempts = 100;
  std::bernoulli_distribution choose_horizontal;
  for (int attempt = 0; attempt < max_attempts; ++attempt) {
    const bool horizontal = choose_horizontal(rng);
    const int width = horizontal ? length : 1;
    const int height = horizontal ? 1 : length;
    const int x = std::uniform_int_distribution<int>(0, size - width)(rng);
    const int y = std::uniform_int_distribution<int>(0, size - height)(rng);

    if (!HasRoom(field, x, y, width, height)) continue;

    for (int row = y; row < y + height; ++row) {
      for (int col = x; col < x + width; ++col) {
        field[row][col] = static_cast<CellType>(length);
      }
    }
    return true;
  }
  return false;
}

bool PlaceFleet(Field& field, std::mt19937& rng) {
  for (int length = 4; length >= 1; --length) {
    for (int ship = 0; ship < ship_counts[length]; ++ship) {
      if (!PlaceShip(field, length, rng)) return false;
    }
  }
  return true;
}
}  // namespace

std::unique_ptr<Field> GeneratePlacement(std::mt19937& rng) {
  constexpr int max_layouts = 100;
  for (int attempt = 0; attempt < max_layouts; ++attempt) {
    auto field = std::make_unique<Field>(
        size, std::vector<CellType>(size, CellType::Empty));
    if (PlaceFleet(*field, rng)) {
      Validate::ValidatePlacement(*field);
      return field;
    }
  }
  throw ValidationFailError(
      "Не удалось сгенерировать расстановку за отведённое число попыток");
}
