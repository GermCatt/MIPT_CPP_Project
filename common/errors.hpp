#pragma once

#include <stdexcept>
#include <string>

class CommonError : public std::runtime_error {
 public:
  explicit CommonError(const std::string& message)
      : std::runtime_error(message) {}
};

class InvalidArgumentError : public CommonError {
 public:
  explicit InvalidArgumentError(const std::string& message)
      : CommonError(message) {}
};

class InvalidMatrixSizeError : public CommonError {
 public:
  explicit InvalidMatrixSizeError(const std::string& message)
      : CommonError(message) {}
};