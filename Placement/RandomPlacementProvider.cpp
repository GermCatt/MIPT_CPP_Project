//
// Created by Леонид Наумов on 22.11.2026.
//

#include "RandomPlacementProvider.hpp"

#include "neural_network/placement_generator.hpp"

RandomPlacementProvider::RandomPlacementProvider()
    : rng_(std::random_device{}()) {}

RandomPlacementProvider::RandomPlacementProvider(std::uint32_t seed) : rng_(seed) {}

std::unique_ptr<Field> RandomPlacementProvider::CreatePlacement() {
    return GeneratePlacement(rng_);
}
