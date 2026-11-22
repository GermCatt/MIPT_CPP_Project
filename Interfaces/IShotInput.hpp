//
// Created by Леонид Наумов on 22.11.2026.
//

#ifndef CPP_PROJECT_ISHOTINPUT_HPP
#define CPP_PROJECT_ISHOTINPUT_HPP

#include "common/game_types.hpp"
#include "common/player_view.hpp"

class IShotInput {
public:
    virtual ~IShotInput() = default;
    virtual Position GetShot(PlayerId player_id, PlayerView View) = 0;
};
#endif //CPP_PROJECT_ISHOTINPUT_HPP
