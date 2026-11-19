#pragma once

#include <cstddef>
#include <iosfwd>
#include <vector>

#include "common/errors.hpp"

class Matrix {
 private:
  int rows_;
  int cols_;
  std::vector<double> body_;

 public:
  explicit Matrix(int rows, int cols);
  explicit Matrix(const std::vector<std::vector<double>>& body);

  int GetRows() const;
  int GetCols() const;

  const std::vector<double>& GetBody() const;

  double& operator()(int row, int col) noexcept {
    return body_[static_cast<std::size_t>(row) *
                     static_cast<std::size_t>(cols_) +
                 static_cast<std::size_t>(col)];
  }

  const double& operator()(int row, int col) const noexcept {
    return body_[static_cast<std::size_t>(row) *
                     static_cast<std::size_t>(cols_) +
                 static_cast<std::size_t>(col)];
  }

  Matrix& operator+=(const Matrix& other);
  Matrix operator+(const Matrix& other) const;

  Matrix& operator-=(const Matrix& other);
  Matrix operator-(const Matrix& other) const;

  Matrix& operator*=(double scalar);
  Matrix operator*(double scalar) const;

  Matrix& operator*=(const Matrix& other);
  Matrix operator*(const Matrix& other) const;

  Matrix Transposed() const;
};

Matrix operator*(double scalar, const Matrix& matrix);
std::ostream& operator<<(std::ostream& stream, const Matrix& matrix);
