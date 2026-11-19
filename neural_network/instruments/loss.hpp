#pragma once

#include "neural_network/instruments/matrix.hpp"

double MaskedBCEWithLogits(const Matrix& logits, const Matrix& targets,
                           const Matrix& mask);

Matrix MaskedBCEWithLogitsGradient(const Matrix& logits, const Matrix& targets,
                                   const Matrix& mask);