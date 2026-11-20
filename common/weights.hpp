/*
Created by Abelentsev German
21.11.2026
*/

#pragma once

#include <filesystem>

#include "neural_network/neural_network.hpp"

void SaveWeights(const NeuralNetwork& network,
                 const std::filesystem::path& path);
NeuralNetwork LoadWeights(const std::filesystem::path& path);
