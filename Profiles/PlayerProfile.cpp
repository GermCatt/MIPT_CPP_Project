//
// Created by Леонид Наумов on 22.11.2026.
//
#include "PlayerProfile.hpp"

#include <utility>
#include "common/errors.hpp"

PlayerProfile::PlayerProfile(
    std::string name,
    PlayerKind kind
) : name_(std::move(name)), kind_(kind) {
    if (name_.empty()) {
        throw InvalidArgumentError("Имя игрока не может быть пустым");
    }
}

const std::string& PlayerProfile::GetName() const {
    return name_;
}

PlayerKind PlayerProfile::GetKind() const {
    return kind_;
}