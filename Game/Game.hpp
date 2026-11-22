//
// Created by Леонид Наумов on 22.11.2026.
//

#ifndef CPP_PROJECT_GAME_H
#define CPP_PROJECT_GAME_H

#include <memory>
#include "Interfaces/IGameField.hpp"
#include "Interfaces/IGameOutput.hpp"
#include "Interfaces/IShotInput.hpp"
class Game {
private:
    std::unique_ptr<IGameField> field_;
    std::unique_ptr<IGameOutput> output_;
    std::unique_ptr<IShotInput> input_;

    PlayerId TurnOrder = PlayerId::first;

    ShotResult PlayerMove();
    void ChangeTurn(ShotResult result);
    bool IsGameOver() const;
public:
    Game(std::unique_ptr<IGameField> field_,
    std::unique_ptr<IGameOutput> output_,
    std::unique_ptr<IShotInput> input_);

    void Run();
};


#endif //CPP_PROJECT_GAME_H
