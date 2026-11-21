/*
Created by Abelentsev German
21.11.2026
*/

#include "common/weights.hpp"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <random>
#include <string>

#include "common/errors.hpp"

namespace fs = std::filesystem;

namespace {
void WriteLayer(std::ostream& out, const DenseLayer& layer) {
  const Matrix& w = layer.GetWeights();
  const Matrix& b = layer.GetBias();
  out << w.GetRows() << ' ' << w.GetCols() << '\n';
  for (int r = 0; r < w.GetRows(); ++r) {
    for (int c = 0; c < w.GetCols(); ++c) {
      if (!std::isfinite(w(r, c))) throw InvalidValueError("Non-finite weight");
      out << w(r, c) << ' ';
    }
    if (!std::isfinite(b(r, 0))) throw InvalidValueError("Non-finite bias");
    out << b(r, 0) << '\n';
  }
}

DenseLayer ReadLayer(std::istream& in, std::uintmax_t file_size) {
  int rows, cols;
  if (!(in >> rows >> cols) || rows <= 0 || cols <= 0) {
    throw InvalidValueError("Invalid layer dimensions");
  }

  if (static_cast<std::uintmax_t>(rows) >
      file_size / (std::uintmax_t(cols) + 1)) {
    throw InvalidValueError("Layer dimensions exceed weights file size");
  }
  Matrix w(rows, cols), b(rows, 1);
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      if (!(in >> w(r, c)) || !std::isfinite(w(r, c))) {
        throw InvalidValueError("Missing or invalid weight");
      }
    }
    if (!(in >> b(r, 0)) || !std::isfinite(b(r, 0))) {
      throw InvalidValueError("Missing or invalid bias");
    }
  }
  return DenseLayer(w, b);
}
}

void SaveWeights(const NeuralNetwork& network, const fs::path& path) {
  if (path.has_parent_path()) fs::create_directories(path.parent_path());
  fs::path staging = path;
  staging += ".tmp-" + std::to_string(std::random_device{}());
  if (!fs::create_directory(staging))
    throw CommonError("Temporary path is in use");
  const fs::path temporary = staging / "weights";
  try {
    std::ofstream out;
    out.exceptions(std::ios::failbit | std::ios::badbit);
    out.open(temporary);
    out.imbue(std::locale::classic());
    out << "NN_WEIGHTS 1\n"
        << std::setprecision(std::numeric_limits<double>::max_digits10);
    WriteLayer(out, network.GetLayer1());
    WriteLayer(out, network.GetLayer2());
    WriteLayer(out, network.GetOutputLayer());
    out.close();
    fs::rename(temporary, path);
  } catch (...) {
    std::error_code ignored;
    fs::remove(temporary, ignored);
    fs::remove(staging, ignored);
    throw;
  }
  fs::remove(staging);
}

NeuralNetwork LoadWeights(const fs::path& path) {
  std::ifstream in(path);
  if (!in) throw CommonError("Cannot open weights file: " + path.string());
  in.imbue(std::locale::classic());
  std::string signature;
  int version;
  if (!(in >> signature >> version) || signature != "NN_WEIGHTS" ||
      version != 1) {
    throw InvalidValueError("Unknown weights file format");
  }
  const auto size = fs::file_size(path);
  const auto first = ReadLayer(in, size);
  const auto second = ReadLayer(in, size);
  const auto output = ReadLayer(in, size);
  std::string extra;
  if (in >> extra || !in.eof() || in.bad())
    throw InvalidValueError("Invalid end of weights file");
  return NeuralNetwork(first, second, output);
}
