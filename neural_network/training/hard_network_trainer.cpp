/*
Created by Abelentsev German
21.11.2026
*/

#include "neural_network/training/hard_network_trainer.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>

namespace {
constexpr int cells = BoardInput::cells;
}

HardNetworkTrainer::HardNetworkTrainer(HardNetwork network)
    : HardNetwork(std::move(network)) {}

HardNetworkTrainer::HardNetworkTrainer(std::mt19937& rng) {
  std::normal_distribution<float> sample(0, 0.16f);
  for (auto& row : std::span(weights_).first(active_hidden_)) {
    for (auto& value : std::span(row).first(83)) value = sample(rng);
  }
  for (auto& value : std::span(output_).first(active_hidden_))
    value = sample(rng);
}
void HardNetworkTrainer::Widen() {
  if (active_hidden_ == hidden) return;
  for (int i = 0; i < default_hidden; ++i) {
    weights_[i + default_hidden] = weights_[i];
    weights_[i + 2 * default_hidden] = weights_[i];
    const float original = output_[i];
    output_[i] = original * 0.5f;
    output_[i + default_hidden] = original * 0.375f;
    output_[i + 2 * default_hidden] = original * 0.125f;
  }
  active_hidden_ = hidden;
}
void HardNetworkTrainer::EnableGrid() {
  if (!use_fleet_)
    throw std::invalid_argument("Search grid needs a fleet-aware source");
  use_grid_ = true;
}
void HardNetworkTrainer::EnableRegions() {
  if (!use_fleet_)
    throw std::invalid_argument("Region pooling needs a fleet-aware source");
  use_regions_ = true;
}

void HardNetworkTrainer::FitFleetCounts() {
  constexpr std::array fleet_counts{Config::single_deck, Config::double_deck,
                                    Config::three_deck, Config::four_deck};
  std::array<std::array<double, 9>, 5> equations{};
  for (int a = 0; a <= fleet_counts[0]; ++a) {
    for (int b = 0; b <= fleet_counts[1]; ++b) {
      for (int c = 0; c <= fleet_counts[2]; ++c) {
        for (int d = 0; d <= fleet_counts[3]; ++d) {
          const std::array lost{a, b, c, d};
          std::array<double, 5> visible{1, 0, 0, 0, 0};
          for (int length = 1; length <= 4; ++length)
            for (int width = 1; width <= length; ++width)
              visible[width] += (length - width + 1) * lost[length - 1];
          for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 5; ++j)
              equations[i][j] += visible[i] * visible[j];
            for (int k = 0; k < 4; ++k)
              equations[i][5 + k] += visible[i] * (fleet_counts[k] - lost[k]);
          }
        }
      }
    }
  }
  for (int col = 0; col < 5; ++col) {
    int pivot = col;
    for (int i = col + 1; i < 5; ++i)
      if (std::abs(equations[i][col]) > std::abs(equations[pivot][col]))
        pivot = i;
    std::swap(equations[col], equations[pivot]);
    const double divisor = equations[col][col];
    if (std::abs(divisor) < 1e-9)
      throw std::runtime_error("Degenerate fleet training set");
    for (int j = col; j < 9; ++j) equations[col][j] /= divisor;
    for (int i = 0; i < 5; ++i) {
      if (i == col) continue;
      const double factor = equations[i][col];
      for (int j = col; j < 9; ++j)
        equations[i][j] -= factor * equations[col][j];
    }
  }
  for (int k = 0; k < 4; ++k)
    for (int j = 0; j < 5; ++j)
      fleet_head_[k][j] = static_cast<float>(equations[j][5 + k]);
  use_fleet_ = true;
  use_packing_ = true;
}

void HardNetworkTrainer::LearnAvailability(const Features& f, bool available) {
  const float step = 0.02f * (static_cast<float>(available) - Availability(f));
  availability_[0] += step;
  for (int j = 0; j < 6; ++j) availability_[j + 1] += step * f[6 + j];
}

void HardNetworkTrainer::AdjustHead(const Features& direction, float scale) {
  const int inputs = ActiveInputs();
  for (int j = 0; j < inputs; ++j) direct_[j] += scale * direction[j];
}
void HardNetworkTrainer::Learn(const Features& f, bool occupied, float weight,
                               float rate) {
  use_packing_ = true;
  std::array<float, hidden> h;
  const float logit = std::clamp(Forward(f, h), -20.0f, 20.0f);
  const float step =
      rate * weight * (occupied - 1.0f / (1.0f + std::exp(-logit)));
  ApplyLogitGradient(f, h, output_, step);
}

void HardNetworkTrainer::ApplyLogitGradient(
    const Features& f, const std::array<float, hidden>& h,
    const std::array<float, hidden>& outgoing, float step) {
  for (int j = 0; j < feature_count; ++j) direct_[j] += step * f[j];
  for (int n = 0; n < active_hidden_; ++n) {
    const float update = step * outgoing[n] * (h[n] >= 0 ? 1.0f : 0.05f);
    for (int j = 0; j < feature_count; ++j) weights_[n][j] += update * f[j];
    output_[n] += step * h[n];
  }
}

void HardNetworkTrainer::LearnListwise(const FeatureMap& f,
                                       const std::array<float, cells>& targets,
                                       const std::array<float, cells>& costs,
                                       float rate) {
  use_packing_ = true;
  std::array<std::array<float, hidden>, cells> activations;
  std::array<double, cells> logits{};
  double maximum = -std::numeric_limits<double>::infinity(), target_sum = 0;
  for (int i = 0; i < cells; ++i) {
    if (costs[i] == 0) continue;
    logits[i] =
        Forward(f[i], activations[i]) + std::log(static_cast<double>(costs[i]));
    maximum = std::max(maximum, logits[i]);
    target_sum += targets[i];
  }
  if (!(target_sum > 0) || !std::isfinite(maximum))
    throw std::runtime_error("Invalid listwise training position");
  double total = 0;
  for (int i = 0; i < cells; ++i) {
    if (costs[i] == 0) continue;
    logits[i] = std::exp(logits[i] - maximum);
    total += logits[i];
  }
  const auto outgoing = output_;
  for (int i = 0; i < cells; ++i) {
    if (costs[i] == 0) continue;
    const float delta =
        static_cast<float>(targets[i] / target_sum - logits[i] / total);
    ApplyLogitGradient(f[i], activations[i], outgoing, rate * delta);
  }
}

void HardNetworkTrainer::LearnPreference(const Features& a, const Features& b,
                                         int shots_saved, float rate,
                                         const HardNetwork& reference) {
  std::array<float, hidden> ha, hb;
  const float gap = Forward(a, ha) - Forward(b, hb);
  const float original_gap = reference.Score(a) - reference.Score(b);
  const float target = original_gap + 0.5f * std::clamp(shots_saved, -6, 6);
  float norm = 1;
  for (int i = 0; i < feature_count; ++i) norm += (a[i] - b[i]) * (a[i] - b[i]);
  for (int i = 0; i < active_hidden_; ++i)
    norm += (ha[i] - hb[i]) * (ha[i] - hb[i]);
  const float step = rate * (target - gap) / norm;
  for (int i = 0; i < feature_count; ++i) direct_[i] += step * (a[i] - b[i]);
  for (int i = 0; i < active_hidden_; ++i) output_[i] += step * (ha[i] - hb[i]);
}

void HardNetworkTrainer::Save(const std::filesystem::path& path) const {
  std::ofstream out(path);
  Save(out);
  out.close();
  if (!out) throw std::runtime_error("Cannot save local policy");
}
void HardNetworkTrainer::Save(std::ostream& out) const {
  const int encoding = use_grid_ ? 5 : use_fleet_ ? 4 : use_packing_ ? 3 : 2;
  if (use_regions_) {
    out << "LOCAL_POLICY_V7\n" << encoding << ' ' << active_hidden_ << '\n';
  } else if (active_hidden_ != default_hidden) {
    out << "LOCAL_POLICY_V6\n" << encoding << ' ' << active_hidden_ << '\n';
  } else {
    out << "LOCAL_POLICY_V" << encoding << '\n';
  }
  out << std::setprecision(std::numeric_limits<float>::max_digits10);
  auto row = [&](const auto& values) {
    for (float value : values) out << value << ' ';
    out << '\n';
  };
  const int inputs = ActiveInputs();
  for (const auto& weights : std::span(weights_).first(active_hidden_))
    row(std::span(weights).first(inputs));
  row(std::span(direct_).first(inputs));
  row(std::span(output_).first(active_hidden_));
  row(availability_);
  if (use_fleet_) {
    for (const auto& coefficients : fleet_head_) row(coefficients);
  }
  if (!out) throw std::runtime_error("Cannot save local policy");
}
