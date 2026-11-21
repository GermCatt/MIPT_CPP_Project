/*
Created by Abelentsev German
21.11.2026
*/

#pragma once

#include "neural_network/training/training_example.hpp"

void MaskKnownWater(TrainingExample& example);
int HuntScore(const Matrix& input, int cell);
int ChooseNetworkShot(const TrainingExample& example, const Matrix& scores,
                      const ShotOrder& order, bool finish_hits);
