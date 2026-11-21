/*
Created by Abelentsev German
19.11.2026
*/

/*
Modified by Abelentsev German
21.11.2026
*/
#pragma once

#include "neural_network/instruments/matrix.hpp"

double MaskedBCEWithLogits(const Matrix& logits, const Matrix& targets,
                           const Matrix& mask);

Matrix MaskedBCEWithLogitsGradient(const Matrix& logits, const Matrix& targets,
                                   const Matrix& mask);