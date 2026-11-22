//
// Created by Леонид Наумов on 22.11.2026.
//

#ifndef CPP_PROJECT_IPROFILEINPUT_HPP
#define CPP_PROJECT_IPROFILEINPUT_HPP

#include <string>

class IProfileInput {
public:
    virtual ~IProfileInput() = default;
    virtual std::string GetHumanName() = 0;
};

#endif //CPP_PROJECT_IPROFILEINPUT_HPP
