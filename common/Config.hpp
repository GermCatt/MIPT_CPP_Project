/*
Created by Abelentsev German
21.11.2026
*/

#pragma once

namespace Config {
constexpr int board_size = 10;
constexpr int single_deck = 4;
constexpr int double_deck = 3;
constexpr int three_deck = 2;
constexpr int four_deck = 1;

static_assert(board_size > 4, "Некорректный размер доски");
static_assert(single_deck >= 0, "Количества кораблей заданы некорректно");
static_assert(double_deck >= 0, "Количества кораблей заданы некорректно");
static_assert(three_deck >= 0, "Количества кораблей заданы некорректно");
static_assert(four_deck >= 0, "Количества кораблей заданы некорректно");
static_assert(single_deck + double_deck + three_deck + four_deck > 0,
              "На доске должен быть хотя бы один корабль");

}  // namespace Config
