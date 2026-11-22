/*
Created by Abelentsev German
21.11.2026
*/

/*
Modified by Abelentsev German
22.11.2026
*/

#include "neural_network/hard_network.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <string>

#include "neural_network/local_wound.hpp"

namespace {
constexpr int side = Config::board_size;
constexpr int cells = BoardInput::cells;
constexpr int unknown = 0, miss = 1, hit = 2, sunk = 3;
constexpr int pad = 4, stride = side + 2 * pad;
constexpr int address(int x, int y) { return (y + pad) * stride + x + pad; }
constexpr std::array<std::array<int, 2>, 10> orbits{{{1, 0},
                                                     {1, 1},
                                                     {2, 0},
                                                     {2, 1},
                                                     {2, 2},
                                                     {3, 0},
                                                     {3, 1},
                                                     {3, 2},
                                                     {3, 3},
                                                     {4, 0}}};
struct Orbit {
  std::array<int, 8> offsets{};
  int count = 0;
};
constexpr auto MakeOrbits() {
  std::array<Orbit, orbits.size()> all{};
  for (std::size_t i = 0; i < orbits.size(); ++i) {
    auto& group = all[i];
    for (int dy = -4; dy <= 4; ++dy) {
      for (int dx = -4; dx <= 4; ++dx) {
        const int ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
        if (std::max(ax, ay) == orbits[i][0] &&
            std::min(ax, ay) == orbits[i][1])
          group.offsets[group.count++] = dy * stride + dx;
      }
    }
  }
  return all;
}
constexpr auto groups = MakeOrbits();

constexpr int kinds = 7;
constexpr int integral_side = side + 1;
constexpr int HullLength(int kind) { return (kind + 1) / 2 + 1; }
constexpr int HullWidth(int kind) { return kind % 2 ? HullLength(kind) : 1; }
constexpr int HullHeight(int kind) { return kind % 2 ? 1 : HullLength(kind); }
struct Rectangle {
  int tl, tr, bl, br;
};
template <typename Value, int Side>
class SummedAreaTable final {
 public:
  void Set(int index, Value value) { values_[index] = value; }

  void Build() {
    for (int y = 1; y <= Side; ++y) {
      Value sum{};
      for (int x = 1; x <= Side; ++x) {
        const int index = y * stride + x;
        sum += values_[index];
        values_[index] = sum + values_[index - stride];
      }
    }
  }

  Value Sum(const Rectangle& r) const {
    return values_[r.br] - values_[r.tr] - values_[r.bl] + values_[r.tl];
  }

 private:
  static constexpr int stride = Side + 1;
  std::array<Value, stride * stride> values_{};
};

template <typename Value, std::size_t N>
Value DotPrefix(const std::array<Value, N>& left,
                const std::array<Value, N>& right, std::size_t count) {
  return std::inner_product(left.begin(), left.begin() + count, right.begin(),
                            Value{});
}

struct Segment {
  int cell, padded, length, kind, step, padded_step, integral;
  std::array<Rectangle, kinds> incompatible;
};
constexpr int segment_count = cells + 2 * side * (3 * side - 6);
auto MakeSegments() {
  std::array<Segment, segment_count> all{};
  int n = 0;
  for (int family = 0; family < kinds; ++family) {
    const int width = HullWidth(family), height = HullHeight(family);
    for (int y = 0; y + height <= side; ++y) {
      for (int x = 0; x + width <= side; ++x) {
        auto& s = all[n++];
        s.cell = y * side + x;
        s.padded = address(x, y);
        s.length = HullLength(family);
        s.kind = family;
        s.step = width > 1 ? 1 : side;
        s.padded_step = width > 1 ? 1 : stride;
        s.integral = (y + 1) * integral_side + x + 1;
        for (int k = 0; k < kinds; ++k) {
          const int x0 = std::max(0, x - HullWidth(k));
          const int y0 = std::max(0, y - HullHeight(k));
          const int x1 = std::min(side, x + width + 1);
          const int y1 = std::min(side, y + height + 1);
          s.incompatible[k] = {y0 * integral_side + x0, y0 * integral_side + x1,
                               y1 * integral_side + x0,
                               y1 * integral_side + x1};
        }
      }
    }
  }
  return all;
}
const auto segments = MakeSegments();
constexpr std::array<int, 4> fleet_counts{
    Config::single_deck, Config::double_deck, Config::three_deck,
    Config::four_deck};

void PoolRegions(HardNetwork::FeatureMap& map) {
  std::array<bool, cells> open{}, visited{};
  int total = 0;
  for (int i = 0; i < cells; ++i) {
    open[i] = map[i][0] != 0 && map[i][47] >= 0.5f;
    total += open[i];
  }
  std::array<int, cells> component{};
  for (int start = 0; start < cells; ++start) {
    if (!open[start] || visited[start]) continue;
    visited[start] = true;
    component[0] = start;
    int size = 1, degree = 0, sum_x = 0, sum_y = 0;
    int left = side, right = 0, top = side, bottom = 0;
    std::array<float, 3> density{};
    for (int scanned = 0; scanned < size; ++scanned) {
      const int cell = component[scanned], x = cell % side, y = cell / side;
      left = std::min(left, x);
      right = std::max(right, x);
      top = std::min(top, y);
      bottom = std::max(bottom, y);
      sum_x += x;
      sum_y += y;
      for (int k = 0; k < 3; ++k) density[k] += map[cell][48 + k];
      auto visit = [&](int neighbour) {
        if (!open[neighbour]) return;
        ++degree;
        if (!visited[neighbour]) {
          visited[neighbour] = true;
          component[size++] = neighbour;
        }
      };
      if (x > 0) visit(cell - 1);
      if (x + 1 < side) visit(cell + 1);
      if (y > 0) visit(cell - side);
      if (y + 1 < side) visit(cell + side);
    }
    const float inverse = 1.0f / size;
    const int width = right - left + 1, height = bottom - top + 1;
    for (int j = 0; j < size; ++j) {
      const int cell = component[j];
      auto& f = map[cell];
      f[103] = static_cast<float>(size) / total;
      f[104] = 0.25f * degree * inverse;
      for (int k = 0; k < 3; ++k) f[105 + k] = density[k] * inverse;
      f[108] = static_cast<float>(std::min(width, height)) / side;
      f[109] = inverse;
      f[110] = (std::abs(cell % side - sum_x * inverse) +
                std::abs(cell / side - sum_y * inverse)) /
               (width + height);
    }
  }
}

void PoolGrid(HardNetwork::FeatureMap& map,
              const std::array<float, 4>& remaining, float phase) {
  for (int length = 2; length <= 4; ++length) {
    const float gate = std::clamp(remaining[length - 1], 0.0f, 1.0f);
    if (gate == 0) continue;
    std::array<float, 8> costs{};
    std::array<float, cells> support{};
    for (int i = 0; i < cells; ++i) {
      const auto& f = map[i];
      support[i] = std::min(1.0f, 4 * f[48 + length - 2]);
      const float cost = f[47] * support[i];
      const int x = i % side, y = i / side;
      costs[(x + y) % length] += cost;
      costs[length + (x - y + 12) % length] += cost;
    }
    const int best = static_cast<int>(
        std::min_element(costs.begin(), costs.begin() + 2 * length) -
        costs.begin());
    float mean = 0;
    for (int j = 0; j < length; ++j) mean += costs[j] / length;
    if (mean <= 1e-7f) continue;
    for (int i = 0; i < cells; ++i) {
      if (support[i] == 0) continue;
      const int x = i % side, y = i / side;
      const int a = (x + y) % length, b = length + (x - y + 12) % length;
      const float deficit = (mean - std::min(costs[a], costs[b])) / mean;
      const float affinity =
          gate * support[i] *
          ((a == best || b == best) + std::clamp(deficit, -1.0f, 1.0f));
      map[i][91 + length - 2] = affinity * (1 - phase);
      map[i][94 + length - 2] = affinity * phase;
    }
  }
}

float FleetSurvival(float survival, float count) {
  float product = 1;
  const int whole = static_cast<int>(count);
  for (int ship = 0; ship < whole; ++ship) product *= survival;
  return product * (1 - (count - whole) * (1 - survival));
}

void PoolPacking(const std::array<float, stride * stride>& open, float area,
                 HardNetwork::FeatureMap& map,
                 const std::array<float, 4>* remaining) {
  std::array<SummedAreaTable<float, side>, kinds> integral{};
  std::array<float, 4> mass{}, discounted_mass{};
  std::array<float, 4> fleet_mass{};
  std::array<float, segment_count> supports{};
  for (std::size_t i = 0; i < segments.size(); ++i) {
    const auto& s = segments[i];
    float support = open[s.padded];
    for (int deck = 1; deck < s.length; ++deck)
      support *= open[s.padded + deck * s.padded_step];
    supports[i] = support;
    integral[s.kind].Set(s.integral, support);
    mass[s.length - 1] += support;
  }
  for (auto& table : integral) table.Build();
  std::array<float, 4> inverse{};
  for (int k = 0; k < 4; ++k) inverse[k] = 1.0f / (mass[k] + 1);
  for (std::size_t i = 0; i < segments.size(); ++i) {
    const float support = supports[i];
    if (support == 0) continue;
    const auto& s = segments[i];
    std::array<float, 4> conflict{};
    for (int k = 0; k < kinds; ++k) {
      const auto& t = integral[k];
      const auto& r = s.incompatible[k];
      conflict[HullLength(k) - 1] += t.Sum(r);
    }
    float survival = 1, fleet_survival = 1;
    for (int k = 0; k < 4; ++k) {
      const float lost_mass = conflict[k] - (k + 1 == s.length ? support : 0);
      survival *= 1 - std::clamp(lost_mass * inverse[k], 0.0f, 0.98f);
      if (remaining) {
        const float competitors =
            std::max(0.0f, (*remaining)[k] - (k + 1 == s.length));
        const float free_fraction =
            1 - std::clamp(conflict[k] / std::max(1e-8f, mass[k]), 0.0f, 1.0f);
        fleet_survival *= FleetSurvival(free_fraction, competitors);
      }
    }
    const float packed = support * survival;
    const float fleet_packed = support * fleet_survival;
    float fringe = 0;
    if (remaining) {
      const auto& t = integral[0];
      const auto& r = s.incompatible[0];
      fringe = t.Sum(r);
      for (int deck = 0; deck < s.length; ++deck)
        fringe -= open[s.padded + deck * s.padded_step];
      fleet_mass[s.length - 1] += fleet_packed;
    }
    discounted_mass[s.length - 1] += packed;
    for (int deck = 0; deck < s.length; ++deck) {
      auto& f = map[s.cell + deck * s.step];
      f[65 + s.length - 1] += support;
      f[69 + s.length - 1] += packed;
      if (remaining) {
        f[83 + s.length - 1] += fleet_packed;
        f[87 + s.length - 1] += fleet_packed * std::max(0.0f, fringe) / 10;
      }
    }
  }
  std::array<float, 4> fleet_scale{};
  if (remaining) {
    float decks_left = 0;
    for (int k = 0; k < 4; ++k) decks_left += (k + 1) * (*remaining)[k];
    for (int k = 0; k < 4; ++k)
      fleet_scale[k] =
          area * (*remaining)[k] /
          (std::max(1.0f, decks_left) * std::max(1e-8f, fleet_mass[k]));
  }
  for (auto& f : map) {
    if (f[0] == 0) continue;
    for (int k = 0; k < 4; ++k) {
      const float regularizer = 0.1f;
      f[73 + k] = f[69 + k] / (regularizer + f[65 + k]);
      f[65 + k] =
          std::min(4.0f, f[65 + k] * area / (regularizer + (k + 1) * mass[k]));
      f[69 + k] =
          std::min(4.0f, f[69 + k] * area /
                             (regularizer + (k + 1) * discounted_mass[k]));
      if (remaining) {
        f[83 + k] = std::min(8.0f, f[83 + k] * fleet_scale[k]);
        f[87 + k] = std::min(8.0f, f[87 + k] * fleet_scale[k]);
      }
    }
  }
}
}  // namespace
float HardNetwork::Availability(const Features& f) const {
  float logit = availability_[0];
  for (int j = 0; j < 6; ++j) logit += availability_[j + 1] * f[6 + j];
  return 1.0f / (1.0f + std::exp(-std::clamp(logit, -18.0f, 18.0f)));
}
HardNetwork::FeatureMap HardNetwork::Encode(const Matrix& input) const {
  if (input.GetRows() != cells * 4 || input.GetCols() != 1)
    throw std::invalid_argument("Expected four visible channels per cell");
  std::array<int, stride * stride> board;
  board.fill(4);
  std::array<int, 4> totals{};
  for (int i = 0; i < cells; ++i) {
    int state = -1;
    for (int channel = 0; channel < NetworkInput::features_per_cell;
         ++channel) {
      const double value =
          input(i * NetworkInput::features_per_cell + channel, 0);
      if (value == 0) continue;
      if (value != 1 || state >= 0)
        throw std::invalid_argument("Invalid visible state");
      state = channel;
    }
    if (state < 0) throw std::invalid_argument("Invalid visible state");
    board[address(i % side, i / side)] = state;
    ++totals[state];
  }
  FeatureMap features{};
  for (int i = 0; i < cells; ++i) {
    const int x = i % side, y = i / side, center = address(x, y);
    if (board[center] != unknown) continue;
    auto& f = features[i];
    f[0] = 1;
    f[1] = totals[miss] * 0.025f;
    f[2] = std::min(totals[hit], 3);
    f[3] = totals[sunk] * 0.05f;
    const int mx = std::min(x, side - 1 - x), my = std::min(y, side - 1 - y);
    f[4] = std::min(mx, my) * 0.25f;
    f[5] = std::max(mx, my) * 0.25f;
    for (int g = 0; g < static_cast<int>(groups.size()); ++g) {
      std::array<int, 5> counts{};
      for (int k = 0; k < groups[g].count; ++k)
        ++counts[board[center + groups[g].offsets[k]]];
      for (int state = miss; state <= sunk; ++state)
        f[6 + 3 * g + state - 1] = static_cast<float>(counts[state]);
    }
    for (int axis : {1, stride}) {
      for (int length = 2; length <= 4; ++length) {
        for (int left = 0; left < length; ++left) {
          bool clear = true, wounded = false;
          for (int j = -left; j < length - left; ++j) {
            const int state = board[center + axis * j];
            if (state != unknown && state != hit) {
              clear = false;
              break;
            }
            wounded |= state == hit;
          }
          if (clear) f[(wounded ? 39 : 36) + length - 2] += 0.25f;
        }
      }
      for (int d : {-axis, axis}) {
        if (board[center + d] == hit && board[center + 2 * d] == hit)
          f[42] += 1;
        if (board[center + d] == hit && board[center - d] == hit) f[43] += 0.5f;
      }
    }
    f[44] = mx == 0 || my == 0;
    f[45] = mx == 0 && my == 0;
    f[46] = totals[unknown] * 0.01f;
  }
  std::array<float, stride * stride> open{};
  float area = 0;
  for (int i = 0; i < cells; ++i) {
    const int pos = address(i % side, i / side);
    if (board[pos] != unknown) continue;
    features[i][47] = open[pos] = Availability(features[i]);
    area += open[pos];
  }
  std::array<float, 3> total_support{};
  std::array<int, 3> sunk_stretches{};
  for (int axis : {1, stride}) {
    const int across = axis == 1 ? stride : 1;
    const int step = axis == 1 ? 1 : side;
    for (int i = 0; i < cells; ++i) {
      const int start = address(i % side, i / side);
      if (board[start] == sunk) {
        for (int k = 1; k <= 3 && board[start + k * axis] == sunk; ++k)
          ++sunk_stretches[k - 1];
      }
      if (board[start] != unknown) continue;
      const int before = start - axis;
      float surround =
          open[before - across] + open[before] + open[before + across];
      surround += open[start - across] + open[start + across];
      float support = open[start];
      for (int k = 1; k <= 3; ++k) {
        const int pos = start + k * axis;
        if (board[pos] != unknown) break;
        support *= open[pos];
        surround += open[pos - across] + open[pos + across];
        const int end = pos + axis;
        const float exposure =
            surround + open[end - across] + open[end] + open[end + across];
        total_support[k - 1] += support;
        for (int deck = 0; deck <= k; ++deck) {
          auto& f = features[i + deck * step];
          f[48 + k - 1] += support;
          f[54 + k - 1] += support * exposure;
        }
      }
    }
  }
  for (int i = 0; i < cells; ++i) {
    const int pos = address(i % side, i / side);
    if (board[pos] != unknown) continue;
    auto& f = features[i];
    for (int k = 0; k < 3; ++k) {
      const float support = f[48 + k];
      f[51 + k] =
          std::min(4.0f, 24 * support / (2 + (k + 2) * total_support[k]));
      f[54 + k] /= (1 + support) * 10;
      f[48 + k] *= 0.25f;
      f[57 + k] = total_support[k] / (area + 1);
      f[60 + k] = sunk_stretches[k] * 0.2f;
    }
    f[63] = area * 0.01f;
    for (int dy = -1; dy <= 1; ++dy)
      for (int dx = -1; dx <= 1; ++dx)
        if (dy || dx) f[64] += open[pos + dy * stride + dx] * 0.125f;
  }
  if (use_packing_) {
    std::array<float, 4> remaining{};
    if (use_fleet_) {
      const std::array observations{1, totals[sunk], sunk_stretches[0],
                                    sunk_stretches[1], sunk_stretches[2]};
      for (int k = 0; k < 4; ++k) {
        float count = 0;
        for (int j = 0; j < 5; ++j)
          count += fleet_head_[k][j] * observations[j];
        remaining[k] =
            std::clamp(count, 0.0f, static_cast<float>(fleet_counts[k]));
      }
    }
    PoolPacking(open, area, features, use_fleet_ ? &remaining : nullptr);
    if (use_grid_ && totals[hit] == 0)
      PoolGrid(features, remaining, totals[sunk] / 20.0f);
    if (use_regions_ && totals[hit] == 0) PoolRegions(features);
    if (active_hidden_ != default_hidden && use_fleet_ && totals[hit] > 0)
      PoolWound(input, remaining, features);
    for (int period = 2; period <= 4; ++period) {
      std::array<std::array<int, 4>, 2> shots{};
      for (int i = 0; i < cells; ++i) {
        const int x = i % side, y = i / side;
        if (board[address(x, y)] == unknown) continue;
        ++shots[0][(x + y) % period];
        ++shots[1][(x - y + 12) % period];
      }
      for (int i = 0; i < cells; ++i) {
        if (features[i][0] == 0) continue;
        const int x = i % side, y = i / side;
        const int plus = shots[0][(x + y) % period],
                  minus = shots[1][(x - y + 12) % period];
        const float mean = static_cast<float>(cells - totals[unknown]) / period;
        features[i][77 + 2 * (period - 2)] =
            (std::max(plus, minus) - mean) * 0.15f;
        features[i][78 + 2 * (period - 2)] =
            (std::min(plus, minus) - mean) * 0.15f;
      }
    }
  }
  return features;
}

int HardNetwork::ActiveInputs() const {
  if (use_regions_) return feature_count;
  if (active_hidden_ != default_hidden) return 103;
  return use_grid_ ? 97 : use_fleet_ ? 91 : use_packing_ ? 83 : 65;
}

float HardNetwork::Forward(const Features& f,
                           std::array<float, hidden>& h) const {
  const int inputs = ActiveInputs();
  float logit = DotPrefix(direct_, f, inputs);
  for (int n = 0; n < active_hidden_; ++n) {
    const float raw = DotPrefix(weights_[n], f, inputs);
    h[n] = raw >= 0 ? raw : 0.05f * raw;
    logit += output_[n] * h[n];
  }
  return logit;
}
int HardNetwork::Choose(const Matrix& input, const ShotOrder& order) const {
  return ChooseEncoded(input, Encode(input), order);
}
float HardNetwork::Score(const Features& features) const {
  std::array<float, hidden> h;
  return Forward(features, h);
}
int HardNetwork::ChooseEncoded(const Matrix& input, const FeatureMap& features,
                               const ShotOrder& order) const {
  if (input.GetRows() != cells * NetworkInput::features_per_cell ||
      input.GetCols() != 1)
    throw std::invalid_argument("Invalid visible board dimensions");
  std::array<bool, cells> seen{};
  float best = -std::numeric_limits<float>::infinity();
  int chosen = -1;
  for (int i : order) {
    if (i < 0 || i >= cells || seen[i])
      throw std::invalid_argument("Invalid shot order");
    seen[i] = true;
    if (input(i * 4, 0) == 0) continue;
    const float score = Score(features[i]);
    if (!std::isfinite(score))
      throw std::runtime_error("Non-finite Hard score");
    if (score > best) {
      best = score;
      chosen = i;
    }
  }
  if (chosen < 0)
    throw std::runtime_error("No untried cell with a finite score");
  return chosen;
}
HardNetwork HardNetwork::Load(const std::filesystem::path& path) {
  std::ifstream in(path);
  if (!in)
    throw std::runtime_error("Cannot open Hard weights: " + path.string());
  HardNetwork network = Load(in);
  std::string extra;
  if (in >> extra || !in.eof() || in.bad())
    throw std::runtime_error("Invalid end of Hard weights file");
  return network;
}
HardNetwork HardNetwork::Load(std::istream& in) {
  std::string magic;
  if (!(in >> magic) ||
      (magic != "LOCAL_POLICY_V1" && magic != "LOCAL_POLICY_V2" &&
       magic != "LOCAL_POLICY_V3" && magic != "LOCAL_POLICY_V4" &&
       magic != "LOCAL_POLICY_V5" && magic != "LOCAL_POLICY_V6" &&
       magic != "LOCAL_POLICY_V7"))
    throw std::runtime_error("Invalid local policy header");
  HardNetwork result;
  int encoding = magic.back() - '0';
  if (encoding >= 6) {
    result.use_regions_ = encoding == 7;
    const int minimum_encoding = result.use_regions_ ? 4 : 2;
    if (!(in >> encoding >> result.active_hidden_) ||
        encoding < minimum_encoding || encoding > 5 ||
        (result.active_hidden_ != hidden &&
         !(result.use_regions_ && result.active_hidden_ == default_hidden)))
      throw std::runtime_error("Invalid local policy dimensions");
  }
  auto row = [&](std::span<float> values) {
    std::generate(values.begin(), values.end(), [&] {
      float value;
      if (!(in >> value) || !std::isfinite(value))
        throw std::runtime_error("Invalid local policy weight");
      return value;
    });
  };
  result.use_grid_ = encoding == 5;
  result.use_fleet_ = encoding >= 4;
  result.use_packing_ = encoding >= 3;
  const int inputs = encoding == 1 ? 47 : result.ActiveInputs();
  for (auto& weights : std::span(result.weights_).first(result.active_hidden_))
    row(std::span(weights).first(inputs));
  row(std::span(result.direct_).first(inputs));
  row(std::span(result.output_).first(result.active_hidden_));
  if (encoding != 1) row(result.availability_);
  if (result.use_fleet_) {
    for (auto& coefficients : result.fleet_head_) row(coefficients);
  }
  return result;
}
