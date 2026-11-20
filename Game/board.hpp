/*
Created by Abelentsev German
21.11.2026
*/

#pragma once

#include <memory>
#include <vector>

#include "common/CellType.hpp"

using Field = std::vector<std::vector<CellType>>;

class Board {
 public:
  explicit Board(std::unique_ptr<Field>&& input);

  Board(const Board&) = delete;
  Board& operator=(const Board&) = delete;
  Board(Board&&) noexcept = default;
  Board& operator=(Board&&) noexcept = default;

  void Shoot(int x, int y);
  CellType GetCell(int x, int y) const;
  std::weak_ptr<const Field> ObserveField() const noexcept;

 private:
  void CheckCoordinates(int x, int y) const;
  void MarkShipIfSunk(int x, int y, CellType ship);
  std::shared_ptr<Field> body_;
};
