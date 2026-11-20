/*
Created by Abelentsev German
21.11.2026
*/

#pragma once

#include <vector>

#include "common/CellType.hpp"

class Validate {
 public:
  static void ValidateNeihgbours(
      const std::vector<std::vector<CellType>>& field);
  static void ValidatePlacement(
      const std::vector<std::vector<CellType>>& field);
};
