/*
Created by Abelentsev German
21.11.2026
*/

#pragma once

#include <random>

#include "neural_network/hard_network.hpp"

class HardNetworkTrainer final : public HardNetwork {
 public:
  explicit HardNetworkTrainer(std::mt19937& rng);
  explicit HardNetworkTrainer(HardNetwork network);
  void Save(const std::filesystem::path& path) const;
  void Save(std::ostream& out) const;
  void Learn(const Features& features, bool occupied, float weight, float rate);
  void LearnListwise(const FeatureMap& features,
                     const std::array<float, BoardInput::cells>& target_mass,
                     const std::array<float, BoardInput::cells>& costs,
                     float rate);
  void LearnAvailability(const Features& features, bool available);
  void FitFleetCounts();
  void EnableGrid();
  void EnableRegions();
  void Widen();
  void AdjustHead(const Features& direction, float scale);
  void LearnPreference(const Features& alternative, const Features& original,
                       int shots_saved, float rate,
                       const HardNetwork& reference);

 private:
  void ApplyLogitGradient(const Features& f,
                          const std::array<float, hidden>& activations,
                          const std::array<float, hidden>& outgoing,
                          float step);
};
