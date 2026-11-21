/*
Created by Abelentsev German
21.11.2026
*/

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "neural_network/placement_generator.hpp"
#include "neural_network/training/hard_network_trainer.hpp"
#include "neural_network/training/layout_io.hpp"
#include "neural_network/training/shot_policy.hpp"

namespace {
constexpr int side = Config::board_size, cells = BoardInput::cells;
struct Ship {
  std::vector<int> decks, neighbours;
};
struct Fleet {
  std::vector<Ship> ships;
  std::array<int, cells> owner;
};
Fleet Identify(const Field& field) {
  Fleet fleet;
  fleet.owner.fill(-1);
  for (int y = 0; y < side; ++y) {
    for (int x = 0; x < side; ++x) {
      const int i = y * side + x;
      if (field[y][x] == CellType::Empty || fleet.owner[i] >= 0) continue;
      const int id = static_cast<int>(fleet.ships.size());
      const int length = static_cast<int>(field[y][x]);
      const bool along = x + 1 < side && field[y][x + 1] == field[y][x];
      Ship ship;
      std::array<bool, cells> seen{};
      for (int j = 0; j < length; ++j) {
        const int dx = x + (along ? j : 0), dy = y + (along ? 0 : j);
        const int deck = side * dy + dx;
        fleet.owner[deck] = id;
        ship.decks.push_back(deck);
        for (int ny = std::max(0, dy - 1); ny <= std::min(side - 1, dy + 1);
             ++ny) {
          for (int nx = std::max(0, dx - 1); nx <= std::min(side - 1, dx + 1);
               ++nx) {
            const int cell = side * ny + nx;
            if (seen[cell] || field[ny][nx] != CellType::Empty) continue;
            ship.neighbours.push_back(cell);
            seen[cell] = true;
          }
        }
      }
      fleet.ships.push_back(std::move(ship));
    }
  }
  return fleet;
}
int PositiveInt(std::string_view arg) {
  int number = 0;
  const auto [end, err] =
      std::from_chars(arg.data(), arg.data() + arg.size(), number);
  if (err != std::errc{} || end != arg.data() + arg.size() || number < 1)
    throw std::invalid_argument("Expected a positive integer");
  return number;
}

int Challenger(const HardNetwork& model, const Matrix& input,
               const HardNetwork::FeatureMap& f, const ShotOrder& order,
               int first, std::mt19937& rng) {
  const float floor = model.Score(f[first]) - 1.0f;
  std::vector<std::pair<float, int>> candidates;
  for (int cell : order) {
    if (input(4 * cell, 0) == 0 || f[cell] == f[first]) continue;
    const float score = model.Score(f[cell]);
    if (score >= floor) candidates.emplace_back(score, cell);
  }
  std::stable_sort(
      candidates.begin(), candidates.end(),
      [](const auto& a, const auto& b) { return a.first > b.first; });
  std::vector<int> distinct;
  for (const auto& candidate : candidates) {
    const int cell = candidate.second;
    if (std::none_of(distinct.begin(), distinct.end(),
                     [&](int used) { return f[used] == f[cell]; })) {
      distinct.push_back(cell);
      if (distinct.size() == 6) break;
    }
  }
  if (distinct.empty()) return -1;
  std::uniform_int_distribution<std::size_t> pick(0, distinct.size() - 1);
  return distinct[pick(rng)];
}

bool Fire(Board& board, int shot) {
  board.Shoot(shot % side, shot / side);
  return board.GetCell(shot % side, shot / side) == CellType::Kill;
}

constexpr int fleet_size = Config::single_deck + Config::double_deck +
                           Config::three_deck + Config::four_deck;
struct Branch {
  int turn, shot;
  HardNetwork::Features original, alternative;
};

int Rollout(HardNetworkTrainer& model, const HardNetwork& reference,
            const Field& field, std::mt19937& rng, float rate) {
  const HardNetwork player = model;
  ShotOrder order, trajectory{};
  std::iota(order.begin(), order.end(), 0);
  std::shuffle(order.begin(), order.end(), rng);
  const std::array sample_turns{
      std::uniform_int_distribution<int>(0, 17)(rng),
      std::uniform_int_distribution<int>(18, 36)(rng),
      std::uniform_int_distribution<int>(37, 54)(rng)};
  int moves = 0, alive = fleet_size;
  std::vector<Branch> branches;
  Board baseline(std::make_unique<Field>(field));
  while (alive > 0) {
    const auto example = MakeTrainingExample(baseline);
    const auto features = player.Encode(example.input);
    const int first = player.ChooseEncoded(example.input, features, order);
    if (std::find(sample_turns.begin(), sample_turns.end(), moves) !=
        sample_turns.end()) {
      const int alt =
          Challenger(player, example.input, features, order, first, rng);
      if (alt >= 0)
        branches.push_back({moves, alt, features[first], features[alt]});
    }
    trajectory[moves++] = first;
    if (Fire(baseline, first)) --alive;
  }
  for (const auto& sample : branches) {
    Board branch(std::make_unique<Field>(field));
    int left = fleet_size;
    for (int t = 0; t < sample.turn; ++t) {
      if (Fire(branch, trajectory[t])) --left;
    }
    if (Fire(branch, sample.shot)) --left;
    int counterfactual_moves = sample.turn + 1;
    while (left > 0) {
      const auto example = MakeTrainingExample(branch);
      if (Fire(branch, player.Choose(example.input, order))) --left;
      ++counterfactual_moves;
    }
    model.LearnPreference(sample.alternative, sample.original,
                          moves - counterfactual_moves, rate, reference);
  }
  return static_cast<int>(branches.size());
}

struct Utility {
  float exposure = 0.16f;
  float length = 0;
  float singleton = 0;
  float finish = 8;
  float miss_halo = 1;
  float miss_open = 1;
  std::optional<float> finish_miss{};
  float finish_mix = 0;
};

struct TestBoard {
  Field field;
  ShotOrder order;
};

std::vector<TestBoard> SampleBoards(int count, unsigned seed) {
  std::mt19937 boards(seed), ties(seed ^ 0x9e3779b9u);
  std::vector<TestBoard> sample;
  sample.reserve(count);
  for (int i = 0; i < count; ++i) {
    auto placement = GeneratePlacement(boards);
    ShotOrder order;
    std::iota(order.begin(), order.end(), 0);
    std::shuffle(order.begin(), order.end(), ties);
    sample.push_back({std::move(*placement), order});
  }
  return sample;
}

long long ShotTotal(const HardNetwork& model,
                    const std::vector<TestBoard>& boards) {
  long long shots = 0;
  for (const auto& sample : boards) {
    Board board(std::make_unique<Field>(sample.field));
    int alive = fleet_size;
    while (alive > 0) {
      const int target =
          model.Choose(MakeTrainingExample(board).input, sample.order);
      if (Fire(board, target)) --alive;
      ++shots;
    }
  }
  return shots;
}

HardNetwork::Features HeadDirection(int group, std::mt19937& rng) {
  HardNetwork::Features delta{};
  switch (group) {
    case 0:
      for (int k = 0; k < 4; ++k) {
        delta[69 + k] = (k - 1.5f) * 0.35f;
        delta[83 + k] = (k - 1.5f) * 0.7f;
      }
      break;
    case 1:
      for (int k = 0; k < 3; ++k) delta[54 + k] = 1;
      for (int k = 0; k < 4; ++k) delta[87 + k] = 1;
      break;
    case 2:
      for (int k = 0; k < 4; ++k) {
        delta[65 + k] = -0.65f;
        delta[69 + k] = 0.65f;
      }
      break;
    case 3:
      for (int k = 0; k < 3; ++k) {
        delta[48 + k] = -0.45f;
        delta[51 + k] = 0.9f;
      }
      break;
    case 4:
      delta[4] = -0.5f;
      delta[44] = 1;
      delta[45] = 0.4f;
      break;
    case 5:
      delta[7] = 0.5f;
      delta[39] = 0.7f;
      delta[40] = 0.8f;
      delta[42] = 1;
      break;
    default: {
      constexpr std::array used{
          4,  5,  7,  10, 13, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 47, 51,
          52, 53, 54, 55, 56, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75,
          76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90};
      std::uniform_int_distribution<std::size_t> pick(0, used.size() - 1);
      for (int i = 0; i < 3; ++i)
        delta[used[pick(rng)]] = (rng() & 1u) ? 1.0f : -1.0f;
    }
  }
  return delta;
}

HardNetwork::Features GridDirection(int group, int round) {
  HardNetwork::Features direction{};
  if (group != 6) {
    const int length_index = round % 3;
    if (round % 9 < 6) direction[91 + length_index] = 1;
    if (round % 9 >= 3) direction[94 + length_index] = 1;
  } else {
    for (int k = 0; k < 3; ++k) {
      direction[91 + k] = round % 2 ? (k + 1) / 3.0f : 1.0f;
      direction[94 + k] = round % 2 ? (3 - k) / 3.0f : 1.0f;
    }
  }
  return direction;
}

void TuneHead(int rounds, int boards, unsigned seed,
              const std::filesystem::path& path, HardNetworkTrainer parent,
              bool grid) {
  if (std::filesystem::exists(path))
    throw std::invalid_argument("Output path already exists");
  if (boards > std::numeric_limits<int>::max() / 3)
    throw std::invalid_argument("Too many boards");
  std::mt19937 rng(seed);
  const auto validation =
      SampleBoards(std::max(128, boards * 3), seed ^ 0xb3572b1du);
  auto champion = parent;
  auto best = ShotTotal(parent, validation);
  champion.Save(path);
  std::cout << "Initial validation mean "
            << static_cast<double>(best) / validation.size() << std::endl;
  int stale = 0;
  for (int i = 0; i < rounds; ++i) {
    const auto training = SampleBoards(boards, seed + (i + 1u) * 0x576af371u);
    auto winner = parent;
    auto train_score = ShotTotal(parent, training);
    constexpr std::array grid_steps{1.25f, 0.45f, 2.4f, 0.22f, 0.75f};
    const float scale =
        grid ? grid_steps[i % grid_steps.size()]
             : std::uniform_real_distribution<float>(0.08f, 0.45f)(rng);
    for (int group : {i % 6, 6}) {
      const auto direction =
          grid ? GridDirection(group, i) : HeadDirection(group, rng);
      for (float step : {scale, -scale}) {
        auto choice = parent;
        choice.AdjustHead(direction, step);
        const auto score = ShotTotal(choice, training);
        if (score < train_score) {
          train_score = score;
          winner = choice;
        }
      }
    }
    parent = winner;
    if ((i + 1) % 3 == 0 || i + 1 == rounds) {
      const auto score = ShotTotal(parent, validation);
      if (score < best) {
        best = score;
        champion = parent;
        champion.Save(path);
        stale = 0;
      } else if (++stale == 2) {
        parent = champion;
        stale = 0;
      }
      std::cout << "Round " << i + 1 << ", validation "
                << static_cast<double>(score) / validation.size() << ", best "
                << static_cast<double>(best) / validation.size() << std::endl;
    }
  }
}

struct Outcome {
  ShotOrder shots{};
  std::array<int, fleet_size> discovered;
  std::array<int, fleet_size> sunk{};
  std::array<int, cells> cleared;
  int turns = 0;
  int last_ship = -1;
  int penultimate_discovery = -1;
  int endgame_misses = 0;
};

Outcome RecordOutcome(const HardNetwork& player, const Field& field,
                      const Fleet& fleet, const ShotOrder& order) {
  Board board(std::make_unique<Field>(field));
  Outcome result;
  result.discovered.fill(cells);
  result.cleared.fill(cells);
  int alive = fleet_size;
  while (alive > 0) {
    const int turn = result.turns++;
    const int shot = player.Choose(MakeTrainingExample(board).input, order);
    result.shots[turn] = shot;
    const int owner = fleet.owner[shot];
    if (owner >= 0)
      result.discovered[owner] = std::min(result.discovered[owner], turn);
    if (Fire(board, shot)) {
      --alive;
      result.sunk[owner] = turn;
    }
  }
  for (std::size_t id = 0; id < fleet.ships.size(); ++id) {
    for (int water : fleet.ships[id].neighbours)
      result.cleared[water] = std::min(result.cleared[water], result.sunk[id]);
  }
  result.last_ship = static_cast<int>(
      std::max_element(result.discovered.begin(), result.discovered.end()) -
      result.discovered.begin());
  for (int id = 0; id < fleet_size; ++id) {
    if (id != result.last_ship)
      result.penultimate_discovery =
          std::max(result.penultimate_discovery, result.discovered[id]);
  }
  for (int turn = result.penultimate_discovery + 1;
       turn < result.discovered[result.last_ship]; ++turn)
    result.endgame_misses += fleet.owner[result.shots[turn]] < 0;
  return result;
}

float Coefficient(std::string_view arg) {
  float value = 0;
  const auto end = arg.data() + arg.size();
  const auto parsed = std::from_chars(arg.data(), end, value);
  if (parsed.ec != std::errc{} || parsed.ptr != end || !std::isfinite(value))
    throw std::invalid_argument("Expected a finite utility coefficient");
  return value;
}

void LearnFinishing(HardNetworkTrainer& network, const Fleet& fleet,
                    const Matrix& visible,
                    const HardNetwork::FeatureMap& features,
                    const ShotOrder& order, std::mt19937& rng, float rate,
                    float positive_weight) {
  std::array<bool, fleet_size> wounded{};
  std::array<bool, cells> nearby{};
  for (int hit = 0; hit < cells; ++hit) {
    if (visible(4 * hit + 2, 0) == 0) continue;
    wounded[fleet.owner[hit]] = true;
    const int x = hit % side, y = hit / side;
    for (int delta = -3; delta <= 3; ++delta) {
      if (x + delta >= 0 && x + delta < side) nearby[hit + delta] = true;
      if (y + delta >= 0 && y + delta < side) nearby[hit + delta * side] = true;
    }
  }
  for (int cell : order) {
    if (visible(4 * cell, 0) == 0) continue;
    const int owner = fleet.owner[cell];
    const bool positive = owner >= 0 && wounded[owner];
    if (positive || nearby[cell] || rng() % 16 == 0)
      network.Learn(features[cell], positive, positive ? positive_weight : 1.0f,
                    rate);
  }
}

void PlayAndTrain(HardNetworkTrainer& network, const Field& field,
                  std::mt19937& rng, float rate, const Utility& utility,
                  const HardNetwork* frozen_search, float finish_positive,
                  bool hindsight, float halo_bonus, float delay_bonus,
                  float critical_bonus, bool listwise) {
  const HardNetwork player = network;
  const Fleet fleet = Identify(field);
  Board board(std::make_unique<Field>(field));
  ShotOrder order;
  std::iota(order.begin(), order.end(), 0);
  std::shuffle(order.begin(), order.end(), rng);
  std::optional<Outcome> outcome;
  if (hindsight) outcome = RecordOutcome(player, field, fleet, order);
  int alive = static_cast<int>(fleet.ships.size());
  std::array<bool, cells> revealed_water{};
  std::array<int, cells> bordering_ships{};
  for (const auto& ship : fleet.ships) {
    for (int cell : ship.neighbours) ++bordering_ships[cell];
  }
  for (int turn = 0; alive > 0; ++turn) {
    auto example = MakeTrainingExample(board);
    bool unresolved_hit = false;
    if (frozen_search) {
      for (int cell = 0; cell < cells && !unresolved_hit; ++cell)
        unresolved_hit = example.input(4 * cell + 2, 0) == 1;
    }
    const auto& acting =
        frozen_search && !unresolved_hit ? *frozen_search : player;
    auto features = acting.Encode(example.input);
    int shot = outcome ? outcome->shots[turn]
                       : acting.ChooseEncoded(example.input, features, order);
    if (!frozen_search && !outcome && rng() % 100 < 5) {
      int random;
      do {
        random = static_cast<int>(rng() % cells);
      } while (!example.input(random * 4, 0));
      shot = random;
    }
    if (frozen_search && unresolved_hit) {
      LearnFinishing(network, fleet, example.input, features, order, rng, rate,
                     finish_positive);
    } else if (!frozen_search && (listwise || turn % 2 == 0)) {
      if (!listwise) MaskKnownWater(example);
      std::vector<float> reward;
      std::array<bool, fleet_size> wounded_ships{};
      bool finishing = false;
      for (const auto& ship : fleet.ships) {
        bool wounded = false;
        for (int deck : ship.decks)
          wounded |= example.input(4 * deck + 2, 0) == 1;
        finishing |= wounded;
        wounded_ships[reward.size()] = wounded;
        int extra = 0;
        for (int cell : ship.neighbours)
          if (!revealed_water[cell] && example.input(4 * cell, 0)) ++extra;
        const int length = static_cast<int>(ship.decks.size());
        const float fraction_afloat = static_cast<float>(alive) / fleet_size;
        float discovery = 0.55f + utility.exposure * extra +
                          utility.length * (length - 1) * fraction_afloat +
                          (length == 1 ? utility.singleton : 0.0f);
        if (outcome) {
          const auto id = reward.size();
          const float delay =
              static_cast<float>(std::max(0, outcome->discovered[id] - turn));
          discovery +=
              delay_bonus * delay / std::max(1, outcome->turns - 1 - turn);
          if (static_cast<int>(id) == outcome->last_ship &&
              turn <= outcome->penultimate_discovery)
            discovery +=
                critical_bonus * std::min(2.0f, outcome->endgame_misses / 6.0f);
        }
        if (!std::isfinite(discovery))
          throw std::invalid_argument("Search reward overflow");
        float finish_value = utility.finish;
        if (wounded && utility.finish_mix > 0) {
          int unhit = 0;
          for (int deck : ship.decks) unhit += example.input(4 * deck, 0) == 1;
          const float value_per_deck = 2.0f + 1.4f * extra / std::max(1, unhit);
          finish_value =
              std::lerp(utility.finish, value_per_deck, utility.finish_mix);
        }
        reward.push_back(wounded ? finish_value : std::max(0.05f, discovery));
      }
      std::array<float, cells> targets{}, costs{};
      const int start = listwise ? 0 : static_cast<int>(rng() % cells);
      for (int offset = 0; offset < cells; offset += listwise ? 1 : 3) {
        const int cell = order[(start + offset) % cells];
        if (example.input(4 * cell, 0) != 1) continue;
        const int ship_id = fleet.owner[cell];
        float miss_weight =
            bordering_ships[cell] > 0 ? utility.miss_halo : utility.miss_open;
        if (finishing && utility.finish_miss)
          miss_weight = *utility.finish_miss;
        if (outcome && !finishing && !revealed_water[cell] &&
            outcome->cleared[cell] < cells) {
          const int wait = outcome->cleared[cell] - turn;
          miss_weight += halo_bonus / (1.0f + wait / 8.0f);
        }
        if (listwise) {
          costs[cell] = ship_id < 0 ? miss_weight : 1.0f;
          if (ship_id >= 0) {
            const float priority =
                finishing && !wounded_ships[ship_id] ? 0.1f : 1.0f;
            targets[cell] = priority * reward[ship_id];
          }
        } else {
          network.LearnAvailability(features[cell], example.mask(cell, 0) == 1);
          network.Learn(features[cell], ship_id >= 0,
                        ship_id >= 0 ? reward[ship_id] : miss_weight, rate);
        }
      }
      if (listwise) network.LearnListwise(features, targets, costs, rate);
    }
    board.Shoot(shot % side, shot / side);
    if (board.GetCell(shot % side, shot / side) == CellType::Kill) {
      --alive;
      for (int water : fleet.ships[fleet.owner[shot]].neighbours) {
        revealed_water[water] = true;
        --bordering_ships[water];
      }
    }
  }
}
}  // namespace

int main(int argc, char** argv) try {
  bool widen = false, fleet_features = false, listwise = false, regions = false;
  std::ifstream layouts;
  while (argc > 1) {
    const std::string_view flag(argv[1]);
    if (flag == "--layouts") {
      if (layouts.is_open() || argc < 3)
        throw std::invalid_argument("Expected --layouts file");
      layouts.open(argv[2]);
      if (!layouts) throw std::invalid_argument("Cannot read training layouts");
      --argc;
      ++argv;
    } else if (flag == "--widen")
      widen = true;
    else if (flag == "--fleet")
      fleet_features = true;
    else if (flag == "--listwise")
      listwise = true;
    else if (flag == "--regions")
      regions = true;
    else
      break;
    --argc;
    ++argv;
  }
  if (listwise && (argc <= 1 || std::string_view(argv[1]) != "--hindsight"))
    throw std::invalid_argument("--listwise requires --hindsight");
  const bool grid = argc > 1 && std::string_view(argv[1]) == "--grid";
  if (grid || (argc > 1 && std::string_view(argv[1]) == "--tune-head")) {
    if (layouts.is_open())
      throw std::invalid_argument("Head tuning uses its own board samples");
    if (argc != 7)
      throw std::invalid_argument(
          "--tune-head/--grid rounds boards seed output source");
    HardNetworkTrainer model(HardNetwork::Load(argv[6]));
    if (widen) model.Widen();
    if (fleet_features) model.FitFleetCounts();
    if (regions) model.EnableRegions();
    if (grid) model.EnableGrid();
    TuneHead(PositiveInt(argv[2]), PositiveInt(argv[3]), PositiveInt(argv[4]),
             argv[5], model, grid);
    return 0;
  }
  const bool rollout = argc > 1 && std::string_view(argv[1]) == "--rollout";
  const bool tune_utility =
      argc > 1 && std::string_view(argv[1]) == "--utility";
  const bool finish = argc > 1 && std::string_view(argv[1]) == "--finish";
  const bool hindsight = argc > 1 && std::string_view(argv[1]) == "--hindsight";
  const int skip = rollout || tune_utility || finish || hindsight ? 1 : 0;
  if ((rollout && argc != 6) || (hindsight && (argc < 8 || argc > 10)) ||
      (finish && argc != 6 && argc != 7) ||
      (tune_utility && argc != 10 && argc != 12 && argc != 13) ||
      (!skip && argc != 4 && argc != 5))
    throw std::invalid_argument(
        "local_train [--fleet] games seed output [source] | --rollout games "
        "seed output source | --finish games seed output source "
        "[positive_weight] | --utility games seed output source "
        "exposure_weight length_bonus singleton_bonus finish_weight "
        "[miss_halo_weight miss_open_weight [finish_miss_weight]] | "
        "--hindsight games seed output source halo_weight delay_weight "
        "[critical_weight [finish_mix]]");
  const float finish_positive =
      finish && argc == 7 ? Coefficient(argv[6]) : 1.0f;
  if (finish_positive <= 0)
    throw std::invalid_argument("Finish positive weight must be positive");
  Utility utility;
  const float halo_bonus = hindsight ? Coefficient(argv[6]) : 0.0f;
  const float delay_bonus = hindsight ? Coefficient(argv[7]) : 0.0f;
  const float critical_bonus =
      hindsight && argc >= 9 ? Coefficient(argv[8]) : 0.0f;
  if (hindsight) {
    if (std::min({halo_bonus, delay_bonus, critical_bonus}) < 0)
      throw std::invalid_argument("Hindsight bonuses must be nonnegative");
    utility = {0.40f, 1.5f, 0.0f, 10.0f};
    if (argc == 10) {
      utility.finish_mix = Coefficient(argv[9]);
      if (utility.finish_mix < 0 || utility.finish_mix > 1)
        throw std::invalid_argument("finish_mix must be between zero and one");
    }
  }
  if (tune_utility) {
    utility = {Coefficient(argv[6]), Coefficient(argv[7]), Coefficient(argv[8]),
               Coefficient(argv[9])};
    if (utility.finish <= 0)
      throw std::invalid_argument("Finish weight must be positive");
    if (argc >= 12) {
      utility.miss_halo = Coefficient(argv[10]);
      utility.miss_open = Coefficient(argv[11]);
      if (std::min(utility.miss_halo, utility.miss_open) < 0.05f)
        throw std::invalid_argument("Both miss weights must be at least 0.05");
      if (argc == 13) {
        utility.finish_miss = Coefficient(argv[12]);
        if (*utility.finish_miss < 0.05f)
          throw std::invalid_argument(
              "Finish miss weight must be at least 0.05");
      }
    }
  }
  const int games = PositiveInt(argv[1 + skip]);
  std::mt19937 boards(PositiveInt(argv[2 + skip])), rng(boards() ^ 0x5bd1e995u);
  const std::filesystem::path path(argv[3 + skip]);
  if (std::filesystem::exists(path))
    throw std::invalid_argument("Output path already exists");
  HardNetworkTrainer model =
      argc >= 5 ? HardNetworkTrainer(HardNetwork::Load(argv[4 + skip]))
                : HardNetworkTrainer(rng);
  const HardNetwork reference = model;
  if (widen) model.Widen();
  if (fleet_features) model.FitFleetCounts();
  if (regions) model.EnableRegions();
  const float initial_rate = listwise ? 0.0026f
                             : argc == 5 || tune_utility || finish || hindsight
                                 ? 0.00065f
                                 : 0.002f;
  int pairs = 0;
  for (int game = 0; game < games; ++game) {
    std::unique_ptr<Field> field;
    if (layouts.is_open()) {
      std::string line;
      if (!std::getline(layouts, line))
        throw std::runtime_error("Too few training layouts");
      if (!line.empty() && line.back() == '\r') line.pop_back();
      field = std::make_unique<Field>(DecodeLayout(line));
    } else {
      field = GeneratePlacement(boards);
    }
    const float decay = 1.0f + 4.0f * game / games;
    if (rollout)
      pairs += Rollout(model, reference, *field, rng, 0.12f / decay);
    else
      PlayAndTrain(model, *field, rng, initial_rate / decay, utility,
                   finish ? &reference : nullptr, finish_positive, hindsight,
                   halo_bonus, delay_bonus, critical_bonus, listwise);
    if ((game + 1) % 500 == 0)
      std::cout << game + 1 << '/' << games << std::endl;
  }
  if (rollout) std::cout << "Paired continuations: " << pairs << '\n';
  if (layouts.is_open() &&
      (layouts.bad() || layouts.peek() != std::char_traits<char>::eof()))
    throw std::runtime_error("Unreadable or extra training layouts");
  model.Save(path);
} catch (const std::exception& e) {
  std::cerr << "Training failed: " << e.what() << '\n';
  return 1;
}
