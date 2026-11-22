//
// Created by Леонид Наумов on 22.11.2026.
//

#include "Game.hpp"
#include "Interfaces/IGameOutput.hpp"
#include "Interfaces/IGameField.hpp"
#include "Interfaces/IShotInput.hpp"
#include "common/game_types.hpp"
#include "common/errors.hpp"

Game::Game(
    std::unique_ptr<IGameField> field,
    std::unique_ptr<IGameOutput> output,
    std::unique_ptr<IShotInput> input
)
    : field_(std::move(field)),
      output_(std::move(output)),
      input_(std::move(input))
{
    if (!field_ || !output_ || !input_) {
        throw InvalidArgumentError(
            "Game: передана пустая зависимость"
        );
    }
}

ShotResult Game::PlayerMove() {
    PlayerView view = field_->GetPlayerView(TurnOrder);
    Position pos = input_->GetShot(TurnOrder, view);
    ShotResult result = field_->GetShotResult(TurnOrder, pos);
    while (result == ShotResult::AlreadyShot) {
        pos = input_->GetShot(TurnOrder, view);
        result = field_->GetShotResult(TurnOrder, pos);
    }
    PlayerView UpdatePlayerView = field_->GetPlayerView(TurnOrder);
    output_->SetPlayerView(TurnOrder, UpdatePlayerView);
    return result;
}

void Game::ChangeTurn(ShotResult result) {
    if (result == ShotResult::miss) {
        if (TurnOrder == PlayerId::first) {
            TurnOrder = PlayerId::second;
        } else {
            TurnOrder = PlayerId::first;
        }
    }
}

bool Game::IsGameOver() const {
    return field_->IsGameOver(TurnOrder);
}


void Game::Run() {
    while (!IsGameOver()) {
       ShotResult result = PlayerMove();
       ChangeTurn(result);
    };
}