/*
Created by Abelentsev German
19.11.2026
*/

/*
Класс матриц НЕБЕЗОПАСНЫЙ. В нём намеренно вырезаны все проверки для ускорения
вычислений в процессе обучения Использовать его не для самой нейросети, а для
чего угодно ещё НЕ РЕКОМЕНДУЕТСЯ
*/
#include "matrix.hpp"

#include <cstddef>
#include <limits>
#include <ostream>

Matrix::Matrix(int rows, int cols) : rows_(rows), cols_(cols) {
  if (rows <= 0 || cols <= 0) {
    throw InvalidArgumentError("Matrix dimensions must be positive");
  }

  const auto r = static_cast<std::size_t>(rows);
  const auto c = static_cast<std::size_t>(cols);

  if (c > body_.max_size() / r) {
    throw InvalidArgumentError("Matrix is too large");
  }

  body_.resize(r * c, 0.0);
}

Matrix::Matrix(const std::vector<std::vector<double>>& body)
    : rows_(0), cols_(0) {
  if (body.empty()) {
    throw InvalidArgumentError("Matrix must contain at least one row");
  }

  if (body.front().empty()) {
    throw InvalidArgumentError("Matrix must contain at least one column");
  }

  const auto r = body.size();
  const auto c = body.front().size();

  const auto max_dimension =
      static_cast<std::size_t>((std::numeric_limits<int>::max)());

  if (r > max_dimension || c > max_dimension) {
    throw InvalidArgumentError("Matrix dimensions exceed int range");
  }

  for (const auto& row : body) {
    if (row.size() != c) {
      throw InvalidArgumentError("Matrix rows must have equal length");
    }
  }

  if (c > body_.max_size() / r) {
    throw InvalidArgumentError("Matrix is too large");
  }

  rows_ = static_cast<int>(r);
  cols_ = static_cast<int>(c);

  body_.reserve(r * c);

  for (const auto& row : body) {
    body_.insert(body_.end(), row.begin(), row.end());
  }
}

int Matrix::GetRows() const { return rows_; }

int Matrix::GetCols() const { return cols_; }

const std::vector<double>& Matrix::GetBody() const { return body_; }

Matrix& Matrix::operator+=(const Matrix& other) {
  if (rows_ != other.rows_ || cols_ != other.cols_) {
    throw InvalidMatrixSizeError("Size error: matrix sizes are different");
  }

  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      (*this)(i, j) += other(i, j);
    }
  }

  return *this;
}

Matrix Matrix::operator+(const Matrix& other) const {
  Matrix result(*this);
  result += other;
  return result;
}

Matrix& Matrix::operator-=(const Matrix& other) {
  if (rows_ != other.rows_ || cols_ != other.cols_) {
    throw InvalidMatrixSizeError("Size error: matrix sizes are different");
  }

  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      (*this)(i, j) -= other(i, j);
    }
  }

  return *this;
}

Matrix Matrix::operator-(const Matrix& other) const {
  Matrix result(*this);
  result -= other;
  return result;
}

Matrix& Matrix::operator*=(double scalar) {
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      (*this)(i, j) *= scalar;
    }
  }

  return *this;
}

Matrix Matrix::operator*(double scalar) const {
  Matrix result(*this);
  result *= scalar;
  return result;
}

Matrix Matrix::operator*(const Matrix& other) const {
  if (cols_ != other.rows_) {
    throw InvalidMatrixSizeError(
        "Size error: incompatible matrix dimensions "
        "for multiplication");
  }

  Matrix result(rows_, other.cols_);

  for (int i = 0; i < rows_; ++i) {
    for (int k = 0; k < cols_; ++k) {
      const double a_ik = (*this)(i, k);
      for (int j = 0; j < other.cols_; ++j) {
        result(i, j) += a_ik * other(k, j);
      }
    }
  }

  return result;
}

Matrix& Matrix::operator*=(const Matrix& other) {
  *this = *this * other;
  return *this;
}

Matrix Matrix::Transposed() const {
  Matrix result(cols_, rows_);

  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      result(j, i) = (*this)(i, j);
    }
  }

  return result;
}

Matrix operator*(double scalar, const Matrix& matrix) {
  return matrix * scalar;
}

std::ostream& operator<<(std::ostream& stream, const Matrix& matrix) {
  for (int i = 0; i < matrix.GetRows(); ++i) {
    if (i != 0) {
      stream << '\n';
    }

    stream << '[';
    for (int j = 0; j < matrix.GetCols(); ++j) {
      if (j != 0) {
        stream << ' ';
      }
      stream << matrix(i, j);
    }
    stream << ']';
  }

  return stream;
}
