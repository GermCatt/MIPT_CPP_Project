/*
Created by Abelentsev German
22.11.2026
*/

#include "placement_generator.hpp"

#include <array>
#include <vector>

#include "common/Config.hpp"
#include "common/errors.hpp"
#include "common/validate.hpp"

namespace {
constexpr int size = Config::board_size;
constexpr std::array ship_counts{0, Config::single_deck, Config::double_deck,
                                 Config::three_deck, Config::four_deck};
constexpr int fleet_size = Config::single_deck + Config::double_deck +
                           Config::three_deck + Config::four_deck;

struct Placement {
  int left, top, right, bottom;
  CellType type;
};

using Fleet = std::array<Placement, fleet_size>;

auto AllPlacements() {
  std::array<std::vector<Placement>, 5> choices;
  for (int length = 1; length <= 4; ++length) {
    const auto type = static_cast<CellType>(length);
    for (int y = 0; y < size; ++y) {
      for (int x = 0; x < size; ++x) {
        if (x + length <= size) {
          choices[length].push_back({x, y, x + length - 1, y, type});
        }
        if (length > 1 && y + length <= size) {
          choices[length].push_back({x, y, x, y + length - 1, type});
        }
      }
    }
  }
  return choices;
}

bool Touch(const Placement& a, const Placement& b) {
  return a.left <= b.right + 1 && b.left <= a.right + 1 &&
         a.top <= b.bottom + 1 && b.top <= a.bottom + 1;
}

bool DrawFleet(Fleet& fleet, std::mt19937& rng) {
  static const auto choices = AllPlacements();
  int used = 0;
  for (int length = 4; length >= 1; --length) {
    const auto& list = choices[length];
    std::uniform_int_distribution<int> pick(0,
                                            static_cast<int>(list.size()) - 1);
    for (int ship = 0; ship < ship_counts[length]; ++ship) {
      const Placement& candidate = list[pick(rng)];
      for (int i = 0; i < used; ++i) {
        if (Touch(candidate, fleet[i])) return false;
      }
      fleet[used++] = candidate;
    }
  }
  return true;
}
}  // namespace

std::unique_ptr<Field> GeneratePlacement(std::mt19937& rng) {
  constexpr int max_attempts = 250'000;
  Fleet fleet;
  for (int attempt = 0; attempt < max_attempts; ++attempt) {
    if (!DrawFleet(fleet, rng)) continue;

    auto field = std::make_unique<Field>(
        size, std::vector<CellType>(size, CellType::Empty));
    for (const Placement& ship : fleet) {
      for (int y = ship.top; y <= ship.bottom; ++y) {
        for (int x = ship.left; x <= ship.right; ++x) {
          (*field)[y][x] = ship.type;
        }
      }
    }
    Validate::ValidatePlacement(*field);
    return field;
  }
  throw ValidationFailError(
      "Не удалось сгенерировать расстановку за отведённое число попыток");
}
