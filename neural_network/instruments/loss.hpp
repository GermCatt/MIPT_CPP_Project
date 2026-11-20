/*
Created by Abelentsev German
19.11.2026
*/

#pragma once

#include "matrix.hpp"

double MaskedBCEWithLogits(const Matrix& logits, const Matrix& targets,
                           const Matrix& mask);

Matrix MaskedBCEWithLogitsGradient(const Matrix& logits, const Matrix& targets,
                                   const Matrix& mask);