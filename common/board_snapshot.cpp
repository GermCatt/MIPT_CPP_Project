//
// Created by Леонид Наумов on 22.11.2026.
//

#include "board_snapshot.hpp"
#include "Config.hpp"
#include "errors.hpp"

BoardSnapshot::BoardSnapshot(const Field& field)
    :cells_(field) {}

CellType BoardSnapshot::GetCell(Position position) const {
    if (position.x < 0 || position.y < 0 ||
        position.x >= Config::board_size ||
        position.y >= Config::board_size) {
        throw InvalidArgumentError(
            "Координаты клетки за пределами поля"
        );
        }
    return cells_[position.y][position.x];
}
