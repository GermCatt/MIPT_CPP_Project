//
// Created by Леонид Наумов on 22.11.2026.
//

#ifndef CPP_PROJECT_IGAMEFIELD_HPP
#define CPP_PROJECT_IGAMEFIELD_HPP

#include "common/game_types.hpp"
#include "common/board_snapshot.hpp"

class IGameField {
public:
    virtual ~IGameField() = default;

    virtual ShotResult ShotResult(PlayerId player_id, Position position) = 0;
    virtual BoardSnapshot GetSnapshot(PlayerId player_id) = 0;
    virtual bool IsGameOver(PlayerId player_id) = 0;
};

#endif //CPP_PROJECT_IGAMEFIELD_HPP
