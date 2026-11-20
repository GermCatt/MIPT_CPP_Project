#pragma once

#include <stdexcept>
#include <string>

class CommonError : public std::runtime_error {
 public:
  explicit CommonError(const std::string& message)
      : std::runtime_error(message) {}
};

class ValidationFailError : public CommonError {
 public:
  explicit ValidationFailError(const std::string& message)
      : CommonError(message) {}
};


class InvalidArgumentError : public CommonError {
 public:
  explicit InvalidArgumentError(const std::string& message)
      : CommonError(message) {}
};

class MatrixError : public std::runtime_error {
 public:
  explicit MatrixError(const std::string& message)
      : std::runtime_error(message) {}
};

class InvalidMatrixSizeError : public MatrixError {
 public:
  explicit InvalidMatrixSizeError(const std::string& message)
      : MatrixError(message) {}
};

class InvalidBiasSizeError : public MatrixError {
 public:
  explicit InvalidBiasSizeError(const std::string& message)
      : MatrixError(message) {}
};

class NeuroError : public std::runtime_error {
 public:
  explicit NeuroError(const std::string& message)
      : std::runtime_error(message) {}
};

class IncorrectDenseLayerError : public NeuroError {
 public:
  explicit IncorrectDenseLayerError(const std::string& message)
      : NeuroError(message) {}
};

class IncorrectSizesError : public NeuroError {
 public:
  explicit IncorrectSizesError(const std::string& message)
      : NeuroError(message) {}
};

class InvalidValueError : public NeuroError {
 public:
  explicit InvalidValueError(const std::string& message)
      : NeuroError(message) {}
};
