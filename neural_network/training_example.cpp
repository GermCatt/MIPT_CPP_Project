/*
Created by Abelentsev German
22.11.2026
*/

#include "training_example.hpp"

#include "common/Config.hpp"

TrainingExample MakeTrainingExample(const Board& board) {
  constexpr int size = Config::board_size;
  constexpr int cells = TrainingExample::cells;
  constexpr int features = TrainingExample::features_per_cell;
  TrainingExample example{
      .input = Matrix(features * cells, 1),
      .targets = Matrix(cells, 1),
      .mask = Matrix(cells, 1),
  };

  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      const int i = y * size + x;
      const CellType cell = board.GetCell(x, y);
      const bool miss = cell == CellType::Miss;
      const bool hit = cell == CellType::Hit;
      const bool kill = cell == CellType::Kill;

      const bool unknown = !miss && !hit && !kill;
      const int offset = features * i;
      example.input(offset, 0) = unknown;
      example.input(offset + 1, 0) = miss;
      example.input(offset + 2, 0) = hit;
      example.input(offset + 3, 0) = kill;
      example.targets(i, 0) = cell != CellType::Empty && !miss;
      example.mask(i, 0) = unknown;
    }
  }
  return example;
}
