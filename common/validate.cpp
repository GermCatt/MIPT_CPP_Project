/*
Created by Abelentsev German
21.11.2026
*/

#include "validate.hpp"

#include <algorithm>
#include <array>

#include "common/Config.hpp"
#include "common/errors.hpp"

namespace {
using Field = std::vector<std::vector<CellType>>;
using FleetCounts = std::array<int, 5>;
constexpr int size = Config::board_size;

void CheckFieldSize(const Field& field) {
  if (field.size() != size) {
    throw ValidationFailError("Число строк не соответствует размеру поля");
  }
  for (const auto& row : field) {
    if (row.size() != size) {
      throw ValidationFailError("Длина строки не соответствует размеру поля");
    }
  }
}

FleetCounts CheckShipNeighbors(const Field& field) {
  FleetCounts found{};
  std::array<std::array<bool, size>, size> visited{};

  for (int row = 0; row < size; ++row) {
    for (int col = 0; col < size; ++col) {
      const CellType type = field[row][col];
      if (type == CellType::Empty || visited[row][col]) continue;
      if (type < CellType::Ship1 || type > CellType::Ship4) {
        throw ValidationFailError(
            "Недопустимый тип клетки в начальной расстановке");
      }

      const int length = static_cast<int>(type);
      const bool horizontal = col + 1 < size && field[row][col + 1] == type;
      const int bottom = horizontal ? row : row + length - 1;
      const int right = horizontal ? col + length - 1 : col;
      if (bottom >= size || right >= size) {
        throw ValidationFailError("Корабль выходит за границы поля");
      }

      for (int r = row; r <= bottom; ++r) {
        for (int c = col; c <= right; ++c) {
          if (field[r][c] != type) {
            throw ValidationFailError(
                "Палубы корабля должны образовывать непрерывный прямой "
                "отрезок");
          }
          visited[r][c] = true;
        }
      }

      for (int r = std::max(0, row - 1); r <= std::min(size - 1, bottom + 1);
           ++r) {
        for (int c = std::max(0, col - 1); c <= std::min(size - 1, right + 1);
             ++c) {
          const bool inside = r >= row && r <= bottom && c >= col && c <= right;
          if (!inside && field[r][c] != CellType::Empty) {
            throw ValidationFailError(
                "Вокруг корабля должна быть свободная зона");
          }
        }
      }
      ++found[length];
    }
  }
  return found;
}
}

void Validate::ValidateNeihgbours(const Field& field) {
  CheckFieldSize(field);
  CheckShipNeighbors(field);
}

void Validate::ValidatePlacement(const Field& field) {
  CheckFieldSize(field);
  const FleetCounts found = CheckShipNeighbors(field);

  const FleetCounts expected{0, Config::single_deck, Config::double_deck,
                             Config::three_deck, Config::four_deck};
  if (found != expected) {
    throw ValidationFailError(
        "Количество кораблей не соответствует составу флота");
  }
}
