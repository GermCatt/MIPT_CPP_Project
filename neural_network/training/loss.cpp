/*
Created by Abelentsev German
19.11.2026
*/

/*
Modified by Abelentsev German
21.11.2026
*/

#include "neural_network/training/loss.hpp"

#include <algorithm>
#include <cmath>

#include "common/errors.hpp"

double MaskedBCEWithLogits(const Matrix& logits, const Matrix& targets,
                           const Matrix& mask) {
  if (not(logits.GetRows() == targets.GetRows() &&
          targets.GetRows() == mask.GetRows())) {
    throw IncorrectSizesError("Matrix sizes are not equal");
  }
  if (logits.GetCols() != 1 || targets.GetCols() != 1 || mask.GetCols() != 1) {
    throw IncorrectSizesError("Matrixes have more than one coloumn");
  }
  long double sum = 0;
  int count = 0;
  for (int i = 0; i < logits.GetRows(); ++i) {
    if (mask(i, 0) == 1) {
      count += 1;
      sum += std::max(logits(i, 0), 0.0) - logits(i, 0) * targets(i, 0) +
             std::log1p(std::exp(-std::abs(logits(i, 0))));
    }
  }
  if (count == 0) {
    throw InvalidValueError("Mask value is invalid: no cells accounted for");
  }
  return sum / count;
}

Matrix MaskedBCEWithLogitsGradient(const Matrix& logits, const Matrix& targets,
                                   const Matrix& mask) {
  const int rows = logits.GetRows();

  if (targets.GetRows() != rows || mask.GetRows() != rows ||
      logits.GetCols() != 1 || targets.GetCols() != 1 || mask.GetCols() != 1) {
    throw IncorrectSizesError("Expected three column vectors of equal length");
  }

  int count = 0;
  for (int i = 0; i < rows; ++i) {
    if (mask(i, 0) == 1.0) {
      ++count;
    }
  }

  if (count == 0) {
    throw InvalidValueError("Mask selects no cells");
  }

  Matrix gradient(rows, 1);
  for (int i = 0; i < rows; ++i) {
    if (mask(i, 0) != 1.0) {
      continue;
    }

    const double z = logits(i, 0);
    const double y = targets(i, 0);
    const double t = std::exp(-std::abs(z));

    const double derivative = z >= 0.0 ? ((1.0 - y) - y * t) / (1.0 + t)
                                       : ((1.0 - y) * t - y) / (1.0 + t);

    gradient(i, 0) = derivative / count;
  }

  return gradient;
}