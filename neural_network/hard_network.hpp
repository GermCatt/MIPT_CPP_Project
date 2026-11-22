/*
Created by Abelentsev German
21.11.2026
*/

/*
Modified by Abelentsev German
22.11.2026
*/

#pragma once

#include <array>
#include <filesystem>
#include <iosfwd>

#include "neural_network/network_input.hpp"

class HardNetwork {
 public:
  static constexpr int feature_count = 111;
  using Features = std::array<float, feature_count>;
  using FeatureMap = std::array<Features, BoardInput::cells>;

  static HardNetwork Load(const std::filesystem::path& path);
  static HardNetwork Load(std::istream& in);
  FeatureMap Encode(const Matrix& input) const;
  int Choose(const Matrix& visible_input, const ShotOrder& order) const;
  int ChooseEncoded(const Matrix& input, const FeatureMap& features,
                    const ShotOrder& order) const;
  float Score(const Features& features) const;

 private:
  friend class HardNetworkTrainer;
  HardNetwork() = default;
  static constexpr int default_hidden = 12, hidden = 3 * default_hidden;
  int active_hidden_ = default_hidden;
  std::array<Features, hidden> weights_{};
  Features direct_{};
  std::array<float, hidden> output_{};
  std::array<float, 7> availability_{};
  bool use_packing_ = true;
  bool use_fleet_ = false;
  bool use_grid_ = false;
  bool use_regions_ = false;
  std::array<std::array<float, 5>, 4> fleet_head_{};

  int ActiveInputs() const;
  float Availability(const Features& f) const;
  float Forward(const Features& f, std::array<float, hidden>& h) const;
};
