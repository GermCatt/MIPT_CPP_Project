/*
Created by Abelentsev German
21.11.2026
*/

#pragma once

#include <algorithm>
#include <array>
#include <vector>

#include "neural_network/hard_network.hpp"

namespace local_wound {
constexpr int side = Config::board_size;

struct Hull {
  std::array<int, 4> decks{};
  std::array<int, 14> border{};
  int length = 0, border_count = 0;
};

inline auto MakeHulls() {
  std::vector<Hull> hulls;
  for (int length = 2; length <= 4; ++length) {
    for (int vertical = 0; vertical <= 1; ++vertical) {
      const int dx = 1 - vertical, dy = vertical;
      for (int y = 0; y + dy * (length - 1) < side; ++y) {
        for (int x = 0; x + dx * (length - 1) < side; ++x) {
          Hull h;
          h.length = length;
          const int right = x + dx * (length - 1),
                    bottom = y + dy * (length - 1);
          for (int j = 0; j < length; ++j)
            h.decks[j] = (y + j * dy) * side + x + j * dx;
          for (int yy = std::max(0, y - 1);
               yy <= std::min(side - 1, bottom + 1); ++yy) {
            for (int xx = std::max(0, x - 1);
                 xx <= std::min(side - 1, right + 1); ++xx) {
              if (xx < x || xx > right || yy < y || yy > bottom)
                h.border[h.border_count++] = yy * side + xx;
            }
          }
          hulls.push_back(h);
        }
      }
    }
  }
  return hulls;
}

struct Hypothesis {
  std::array<int, 3> unknown{};
  int count = 0, length = 0;
  float support = 1, fringe = 0;
  bool contradicted = false;
};
}  // namespace local_wound

inline void PoolWound(const Matrix& visible,
                      const std::array<float, 4>& remaining,
                      HardNetwork::FeatureMap& features) {
  static const auto hulls = local_wound::MakeHulls();
  std::array<int, BoardInput::cells> state{};
  std::array<float, BoardInput::cells> open{};
  for (int i = 0; i < BoardInput::cells; ++i) {
    state[i] = visible(4 * i, 0) ? 0 : visible(4 * i + 2, 0) ? 2 : 1;
    open[i] = features[i][47];
  }
  std::vector<local_wound::Hypothesis> hypotheses;
  std::array<float, 4> opportunities{};
  for (const auto& hull : hulls) {
    int hidden = 0;
    local_wound::Hypothesis h;
    h.length = hull.length;
    for (int j = 0; j < hull.length; ++j) {
      const int i = hull.decks[j];
      if (state[i] == 1) {
        h.support = 0;
        break;
      }
      if (state[i] == 0) {
        if (hidden < 3) h.unknown[hidden] = i;
        ++hidden;
        h.support *= open[i];
      }
    }
    if (h.support == 0 || hidden == 0) continue;
    opportunities[h.length - 1] += h.support;
    if (hidden == h.length) continue;
    h.count = hidden;
    for (int j = 0; j < hull.border_count; ++j) {
      const int i = hull.border[j];
      h.contradicted |= state[i] == 2;
      h.fringe += open[i];
    }
    hypotheses.push_back(h);
  }

  std::array<float, BoardInput::cells> covered{}, sink{}, immediate_fringe{},
      contradicted{};
  float normalizer = 0, discarded = 0;
  for (auto& h : hypotheses) {
    h.support *= std::clamp(remaining[h.length - 1], 0.0f, 5.0f) /
                 std::max(0.1f, opportunities[h.length - 1]);
    if (h.contradicted)
      discarded += h.support;
    else
      normalizer += h.support;
    for (int k = 0; k < h.count; ++k) {
      const int i = h.unknown[k];
      if (h.contradicted) {
        contradicted[i] += h.support;
        continue;
      }
      covered[i] += h.support;
      if (h.count == 1) {
        sink[i] += h.support;
        immediate_fringe[i] += h.fringe * h.support;
      }
    }
  }
  if (normalizer <= 1e-10f) return;
  const float scale = 1 / normalizer;
  for (int i = 0; i < BoardInput::cells; ++i) {
    auto& f = features[i];
    f[100] = contradicted[i] / (normalizer + discarded);
    if (covered[i] == 0) continue;
    f[97] = covered[i] * scale;
    f[98] = sink[i] * scale;
    f[99] = immediate_fringe[i] * (0.125f * scale);

    std::array<float, BoardInput::cells> both{};
    for (const auto& h : hypotheses) {
      if (h.contradicted ||
          std::find(h.unknown.begin(), h.unknown.begin() + h.count, i) ==
              h.unknown.begin() + h.count)
        continue;
      for (int k = 0; k < h.count; ++k) both[h.unknown[k]] += h.support;
    }
    float follow_miss = 0, follow_hit = 0;
    for (int j = 0; j < BoardInput::cells; ++j) {
      if (j == i) continue;
      follow_miss = std::max(follow_miss, covered[j] - both[j]);
      follow_hit = std::max(follow_hit, both[j]);
    }
    f[101] = follow_miss * scale;
    f[102] = follow_hit * scale;
  }
}
