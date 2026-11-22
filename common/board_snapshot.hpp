
#ifndef CPP_PROJECT_BOARD_SNAPSHOT_H
#define CPP_PROJECT_BOARD_SNAPSHOT_H

#include "CellType.hpp"
#include "game_types.hpp"
#include "Field.hpp"
#include <vector>


class BoardSnapshot {
private:
    Field cells_;
public:
    explicit BoardSnapshot (const Field& field);

    CellType GetCell(Position Position) const;

};

#endif //CPP_PROJECT_BOARD_SNAPSHOT_H
