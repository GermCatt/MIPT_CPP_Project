//
// Created by Леонид Наумов on 22.11.2026.
//

#ifndef CPP_PROJECT_PROFILEREPOSITORY_HPP
#define CPP_PROJECT_PROFILEREPOSITORY_HPP

#include <map>
template<typename Id, typename Profile>
class ProfileRepository final {
private:
    std::map<Id, Profile> profiles_;

public:
    void Register(Id id, Profile profile);

    const Profile* Find(const Id& id) const;

    template<typename Predicate>
    const Profile* FindIf(Predicate predicate) const;
};
#endif //CPP_PROJECT_PROFILEREPOSITORY_HPP
