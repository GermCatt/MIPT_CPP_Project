/*
Created by Abelentsev German
21.11.2026
*/

/*
Modified by Abelentsev German
22.11.2026
*/

#include "board.hpp"

#include <utility>

#include "common/Config.hpp"
#include "common/errors.hpp"
#include "common/validate.hpp"

Board::Board(std::unique_ptr<Field>&& input) {
  if (!input) {
    throw InvalidArgumentError("Указатель на поле пуст");
  }
  Validate::ValidatePlacement(*input);
  body_ = std::move(input);
}

void Board::CheckCoordinates(int x, int y) const {
  if (!body_) {
    throw InvalidArgumentError("Поле доски уже перемещено");
  }
  if (x < 0 || y < 0 || x >= Config::board_size || y >= Config::board_size) {
    throw InvalidArgumentError("Координаты клетки за пределами поля");
  }
}

void Board::Shoot(int x, int y) {
  CheckCoordinates(x, y);
  Field& field = *body_;
  CellType& target = field[y][x];
  if (target == CellType::Miss || target == CellType::Hit ||
      target == CellType::Kill) {
    throw InvalidArgumentError("По этой клетке уже стреляли");
  }
  if (target == CellType::Empty) {
    target = CellType::Miss;
    return;
  }

  const CellType ship = target;
  target = CellType::Hit;
  MarkShipIfSunk(x, y, ship);
}

void Board::MarkShipIfSunk(int x, int y, CellType ship) {
  Field& field = *body_;
  const auto is_deck = [&](int col, int row) {
    if (col < 0 || row < 0 || col >= Config::board_size ||
        row >= Config::board_size) {
      return false;
    }
    return field[row][col] == ship || field[row][col] == CellType::Hit;
  };

  const bool horizontal = is_deck(x - 1, y) || is_deck(x + 1, y);
  const int dx = horizontal ? 1 : 0;
  const int dy = horizontal ? 0 : 1;
  while (is_deck(x - dx, y - dy)) {
    x -= dx;
    y -= dy;
  }

  const int length = static_cast<int>(ship);
  for (int deck = 0; deck < length; ++deck) {
    if (field[y + deck * dy][x + deck * dx] == ship) return;
  }
  for (int deck = 0; deck < length; ++deck) {
    field[y + deck * dy][x + deck * dx] = CellType::Kill;
  }
}

CellType Board::GetCell(int x, int y) const {
  CheckCoordinates(x, y);
  return (*body_)[y][x];
}

BoardInput Board::GetVisibleBoard() const { return BoardInput(*this); }

std::weak_ptr<const Field> Board::ObserveField() const noexcept {
  return body_;
}
