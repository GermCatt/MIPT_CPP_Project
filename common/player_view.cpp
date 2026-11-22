//
// Created by Леонид Наумов on 22.11.2026.
//

#include "player_view.hpp"
#include <utility>

PlayerView::PlayerView(BoardSnapshot own, BoardInput radar)
    : own_board_(std::move(own)),
      radar_board_(std::move(radar)) {}