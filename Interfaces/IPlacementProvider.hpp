//
// Created by Леонид Наумов on 22.11.2026.
//

#ifndef CPP_PROJECT_IPLACEMENTPROVIDER_HPP
#define CPP_PROJECT_IPLACEMENTPROVIDER_HPP
#include "Game/board.hpp"

class IPlacementProvider {
public:
    virtual ~IPlacementProvider() = default;
    virtual std::unique_ptr<Field> CreatePlacement() = 0;
};
#endif //CPP_PROJECT_IPLACEMENTPROVIDER_HPP
