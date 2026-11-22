//
// Created by Леонид Наумов on 22.11.2026.
//

#ifndef CPP_PROJECT_PLAYERPROFILES_HPP
#define CPP_PROJECT_PLAYERPROFILES_HPP
#include <string>
#include "common/PlayerKind.hpp"
class PlayerProfile {
private:
    std::string name_;
    PlayerKind kind_;
public:
    PlayerProfile(std::string name, PlayerKind kind);

    const std::string& GetName() const;
    PlayerKind GetKind() const;
};
#endif //CPP_PROJECT_PLAYERPROFILES_HPP
