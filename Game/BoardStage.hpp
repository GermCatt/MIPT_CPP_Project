//
// Created by Леонид Наумов on 22.11.2026.
//

#ifndef CPP_PROJECT_BOARDSTAGE_HPP
#define CPP_PROJECT_BOARDSTAGE_HPP

#include "board.hpp"
#include "common/game_types.hpp"
#include "common/player_view.hpp"
#include "Interfaces/IGameField.hpp"

class BoardStage : public IGameField {
private:
    std::unique_ptr<Board> first_board_;
    std::unique_ptr<Board> second_board_;

    Board& GetBoard(PlayerId player_id);
    Board& GetRadarBoard(PlayerId player_id);
public:
    BoardStage(std::unique_ptr<Board> first, std::unique_ptr<Board> second);

    ShotResult GetShotResult(PlayerId player_id, Position position) override;
    PlayerView GetPlayerView(PlayerId player_id) override;
    bool IsGameOver(PlayerId player_id) override;
};


#endif //CPP_PROJECT_BOARDSTAGE_HPP
