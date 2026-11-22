//
// Created by Леонид Наумов on 22.11.2026.
//

#ifndef CPP_PROJECT_PLAYER_VIEW_HPP
#define CPP_PROJECT_PLAYER_VIEW_HPP
#include "board_snapshot.hpp"
#include "Game/board_input.hpp"
class PlayerView {
private:
    BoardSnapshot own_board_;
    BoardInput radar_board_;
public:
    PlayerView(BoardSnapshot own, BoardInput radar);
    const BoardSnapshot& GetOwn() const { return own_board_; };
    const BoardInput& GetRadar() const { return radar_board_; };
};
#endif //CPP_PROJECT_PLAYER_VIEW_HPP
