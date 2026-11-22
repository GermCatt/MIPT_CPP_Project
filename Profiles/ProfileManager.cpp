//
// Created by Леонид Наумов on 22.11.2026.
//

#include "ProfileManager.hpp"
#include <string>
#include <utility>
#include "common/errors.hpp"

int ProfileManager::RegisterHuman(IProfileInput& input) {
    std::string name = input.GetHumanName();

    if (profiles_.FindIf([&name](const PlayerProfile& profile) {
        return profile.GetName() == name;
    })) {
        throw InvalidArgumentError(
            "Профиль с таким именем уже существует"
        );
    }

    PlayerProfile profile(
        std::move(name),
        PlayerKind::Human
    );

    int id = next_id_;

    profiles_.Register(id, std::move(profile));

    ++next_id_;

    return id;
}

const PlayerProfile* ProfileManager::Find(int id) const {
    return profiles_.Find(id);
}
