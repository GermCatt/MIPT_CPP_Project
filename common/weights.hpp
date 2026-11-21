/*
Created by Abelentsev German
21.11.2026
*/

#pragma once

#include <filesystem>

#include "neural_network/neural_network.hpp"

inline constexpr char battleship_weights_path[] = "old_models/battleship_leaky_relu_weights.txt";
inline constexpr char ordered_weights_path[] = "old_models/shuffle_experiment/ordered.txt";
inline constexpr char shuffled_weights_path[] = "old_models/shuffle_experiment/shuffled.txt";

void SaveWeights(const NeuralNetwork& network,
                 const std::filesystem::path& path);
NeuralNetwork LoadWeights(const std::filesystem::path& path);
