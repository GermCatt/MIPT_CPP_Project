/*
Created by Abelentsev German
21.11.2026
*/

#pragma once

#include <memory>
#include <random>

#include "Game/board.hpp"

std::unique_ptr<Field> GeneratePlacement(std::mt19937& rng);
