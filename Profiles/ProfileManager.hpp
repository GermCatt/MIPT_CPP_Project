//
// Created by Леонид Наумов on 22.11.2026.
//

#ifndef CPP_PROJECT_PROFILEMENEDGER_HPP
#define CPP_PROJECT_PROFILEMENEDGER_HPP


#include "Interfaces/IProfileInput.hpp"
#include "Profiles/PlayerProfile.hpp"
#include "Profiles/ProfileRepository.hpp"

class ProfileManager final {
private:
    ProfileRepository<int, PlayerProfile> profiles_;
    int next_id_ = 1;

public:
    int RegisterHuman(IProfileInput& input);

    const PlayerProfile* Find(int id) const;
};


#endif //CPP_PROJECT_PROFILEMENEDGER_HPP
