/*
Created by Abelentsev German
21.11.2026
*/

/*
Modified by Abelentsev German
22.11.2026
*/

#include "shot_policy.hpp"

#include <cmath>

#include "common/errors.hpp"

namespace {
constexpr int size = Config::board_size;
}

void MaskKnownWater(TrainingExample& example) {
  for (int cell = 0; cell < BoardInput::cells; ++cell) {
    const int offset = NetworkInput::features_per_cell * cell;
    const bool hit = example.input(offset + 2, 0) == 1;
    const bool sunk = example.input(offset + 3, 0) == 1;
    if (!hit && !sunk) continue;
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dx = -1; dx <= 1; ++dx) {
        if (hit && (dx == 0 || dy == 0)) continue;
        const int x = cell % size + dx, y = cell / size + dy;
        if (x >= 0 && x < size && y >= 0 && y < size)
          example.mask(y * size + x, 0) = 0;
      }
    }
  }
}

int HuntScore(const Matrix& input, int cell) {
  auto hit = [&](int x, int y) {
    return x >= 0 && x < size && y >= 0 && y < size &&
           input(NetworkInput::features_per_cell * (y * size + x) + 2, 0) == 1;
  };
  const int x = cell % size, y = cell / size;
  int score = 0;
  for (int sign : {-1, 1}) {
    if (hit(x + sign, y)) {
      if (hit(x + 2 * sign, y)) return 2;
      score = 1;
    }
    if (hit(x, y + sign)) {
      if (hit(x, y + 2 * sign)) return 2;
      score = 1;
    }
  }
  return score;
}

int ChooseNetworkShot(const TrainingExample& example, const Matrix& scores,
                      const ShotOrder& order, bool finish_hits) {
  int best = -1;
  int best_priority = -1;
  for (int cell : order) {
    if (example.mask(cell, 0) == 0) continue;
    if (!std::isfinite(scores(cell, 0)))
      throw InvalidValueError("Non-finite network score");
    const int priority = finish_hits ? HuntScore(example.input, cell) : 0;
    if (priority > best_priority ||
        (priority == best_priority && scores(cell, 0) > scores(best, 0))) {
      best = cell;
      best_priority = priority;
    }
  }
  if (best == -1) throw InvalidValueError("No available shot");
  return best;
}
