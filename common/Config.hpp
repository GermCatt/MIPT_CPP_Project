#pragma once

namespace Config {
constexpr int board_size = 10;  // Размер доски
constexpr int single_deck = 4;  // Кол-во однопалубных кораблей
constexpr int double_deck = 3;  // Кол-во двухпалубных кораблей
constexpr int three_deck = 2;   // Кол-во трёхпалубных кораблей
constexpr int four_deck = 1;    // Кол-во четырёхпалубных кораблей

static_assert(board_size > 4, "Некорректный размер доски");
static_assert(single_deck >= 0, "Количества кораблей заданы некорректно");
static_assert(double_deck >= 0, "Количества кораблей заданы некорректно");
static_assert(three_deck >= 0, "Количества кораблей заданы некорректно");
static_assert(four_deck >= 0, "Количества кораблей заданы некорректно");
static_assert(single_deck + double_deck + three_deck + four_deck > 0,
              "На доске должен быть хотя бы один корабль");

}  // namespace Config
