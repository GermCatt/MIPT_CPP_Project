/*
Created by Abelentsev German
21.11.2026
*/

/*
Modified by Abelentsev German
22.11.2026
*/

#include "training_example.hpp"

TrainingExample MakeTrainingExample(const Board& board) {
  constexpr int size = Config::board_size;
  constexpr int cells = BoardInput::cells;
  const auto visible = board.GetVisibleBoard();
  TrainingExample example{
      .input = NetworkInput::Encode(visible),
      .targets = Matrix(cells, 1),
      .mask = Matrix(cells, 1),
  };

  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      const int i = y * size + x;
      const CellType cell = board.GetCell(x, y);
      example.targets(i, 0) = cell != CellType::Empty && cell != CellType::Miss;
      example.mask(i, 0) = visible.GetCell(x, y) == BoardInput::Cell::Unknown;
    }
  }
  return example;
}
