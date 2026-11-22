//
// Created by Леонид Наумов on 22.11.2026.
//

#ifndef CPP_PROJECT_RANDOMPLACEMENTPROVIDER_HPP
#define CPP_PROJECT_RANDOMPLACEMENTPROVIDER_HPP


#include <memory>
#include <random>
#include <cstdint>

#include "Interfaces/IPlacementProvider.hpp"

class RandomPlacementProvider final : public IPlacementProvider {
private:
    std::mt19937 rng_;

public:
    RandomPlacementProvider();

    explicit RandomPlacementProvider(std::uint32_t seed);

    std::unique_ptr<Field> CreatePlacement() override;
};

#endif //CPP_PROJECT_RANDOMPLACEMENTPROVIDER_HPP
