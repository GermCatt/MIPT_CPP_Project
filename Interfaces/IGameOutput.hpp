//
// Created by Леонид Наумов on 22.11.2026.
//

#ifndef CPP_PROJECT_IGAMEOUTPUT_HPP
#define CPP_PROJECT_IGAMEOUTPUT_HPP
#include "common/game_types.hpp"
#include "common/player_view.hpp"
class IGameOutput {
public:
    virtual ~IGameOutput() = default;
    virtual void SetPlayerVIew(PlayerId player_id, const PlayerView& result) = 0;
};

#endif //CPP_PROJECT_IGAMEOUTPUT_HPP
