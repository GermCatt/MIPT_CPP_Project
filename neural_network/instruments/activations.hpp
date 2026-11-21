/*
Created by Abelentsev German
19.11.2026
*/

/*
Modified by Abelentsev German
21.11.2026
*/

#pragma once

#include "matrix.hpp"

Matrix ReLU(const Matrix& input);
Matrix LeakyReLU(const Matrix& input);
Matrix LeakyReLU(Matrix&& input);
Matrix LeakyReLUBackward(const Matrix& activated, const Matrix& d_output);

Matrix Sigmoid(const Matrix& input);
