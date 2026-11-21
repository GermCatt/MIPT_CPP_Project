/*
Created by Abelentsev German
21.11.2026
*/

#include "training_example.hpp"

TrainingExample MakeTrainingExample(const Board& board) {
  constexpr int size = Config::board_size;
  constexpr int cells = BoardInput::cells;
  constexpr int features = BoardInput::features_per_cell;
  TrainingExample example{
      .input = Matrix(features * cells, 1),
      .targets = Matrix(cells, 1),
      .mask = Matrix(cells, 1),
  };

  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      const int i = y * size + x;
      const CellType cell = board.GetCell(x, y);
      const int channel = BoardInput::VisibleChannel(cell);
      example.input(features * i + channel, 0) = 1;
      example.targets(i, 0) = cell != CellType::Empty && cell != CellType::Miss;
      example.mask(i, 0) = channel == 0;
    }
  }
  return example;
}
