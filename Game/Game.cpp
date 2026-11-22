//
// Created by Леонид Наумов on 22.11.2026.
//

#include "Game.hpp"
#include "Interfaces/IGameOutput.hpp"
#include "common/game_types.hpp"
void Game::PlayerMove() {
    Position pos = input_->GetShot(TurnOrder);

}

void Game::Run() {
    while (!IsGameOver()) {
       PlayerMove();
       ChangeTurn();
    };
}