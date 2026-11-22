/*
Created by Abelentsev German
21.11.2026
*/

/*
Modified by Abelentsev German
22.11.2026
*/

#pragma once

#include <filesystem>
#include <functional>
#include <utility>

#include "neural_network/network_input.hpp"

class ComputerOpponent {
 public:
  enum class Difficulty { Easy, Hard };

  ComputerOpponent(Difficulty difficulty, unsigned seed,
                   const std::filesystem::path& models = "models");
  std::pair<int, int> ChooseShot(const BoardInput& board) const;

 private:
  ShotOrder order_;
  std::function<int(const Matrix&, const ShotOrder&)> choose_;
};
