//
// Created by Леонид Наумов on 22.11.2026.
//

#include "BoardStage.hpp"
#include <memory>
BoardStage::BoardStage(std::unique_ptr<Board> first, std::unique_ptr<Board> second)
    : first_board_(std::move(first)), second_board_(std::move(second)) {}

Board& BoardStage::GetBoard(PlayerId player) {
    if (player == PlayerId::first) {
        return *first_board_;
    }
    return *second_board_;
}

Board& BoardStage::GetRadarBoard(PlayerId player) {
    if (player == PlayerId::first) {
        return *second_board_;
    }
    return *first_board_;
}

ShotResult BoardStage::GetShotResult(PlayerId player, Position pos) {
    Board& radar = GetRadarBoard(player);

    CellType before = radar.GetCell(pos.x, pos.y);

    if (before == CellType::Miss ||
        before == CellType::Hit ||
        before == CellType::Kill) {
        return ShotResult::AlreadyShot;
        }

    radar.Shoot(pos.x, pos.y);

    CellType after = radar.GetCell(pos.x, pos.y);

    switch (after) {
        case CellType::Miss:
            return ShotResult::miss;

        case CellType::Hit:
            return ShotResult::hit;

        case CellType::Kill:
            return ShotResult::kill;

        default:
            throw std::logic_error("Неизвестный результат выстрела");
    }
}

PlayerView BoardStage::GetPlayerView(PlayerId player_id) {
    Board& own = GetBoard(player_id);
    Board& radar = GetRadarBoard(player_id);

    auto field = own.ObserveField().lock();

    if (!field) {
        throw std::logic_error("Поле игрока недоступно");
    }

    BoardSnapshot own_snapshot{*field};
    BoardInput radar_snapshot = radar.GetVisibleBoard();

    PlayerView result{
        std::move(own_snapshot),
        std::move(radar_snapshot)
    };

    return result;
}

bool BoardStage::IsGameOver(PlayerId player_id) {
    Board& radar = GetRadarBoard(player_id);

    for (int y = 0; y < Config::board_size; ++y) {
        for (int x = 0; x < Config::board_size; ++x) {
            CellType cell = radar.GetCell(x, y);

            if (cell == CellType::Ship1 ||
                cell == CellType::Ship2 ||
                cell == CellType::Ship3 ||
                cell == CellType::Ship4) {
                return false;
                }
        }
    }

    return true;
}