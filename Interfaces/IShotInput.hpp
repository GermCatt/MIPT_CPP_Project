//
// Created by Леонид Наумов on 22.11.2026.
//

#ifndef CPP_PROJECT_ISHOTINPUT_HPP
#define CPP_PROJECT_ISHOTINPUT_HPP

#include "common/game_types.hpp"

class IShotInput {
public:
    virtual ~IShotInput() = default;
    virtual Position GetShot(PlayerId player_id) = 0;
};
#endif //CPP_PROJECT_ISHOTINPUT_HPP
