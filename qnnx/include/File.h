#pragma once

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace qnnx {

bool CheckFileExists(const std::string& filename);

bool CreateDirectory(const std::string& dir_path);

template <typename T>
void ReadFromRawFile(const std::string& file_path, std::vector<T>& buffer) {
  static_assert(std::is_trivially_copyable_v<T>,
                "ReadFromRawFile only supports trivially copyable types");

  std::ifstream file(file_path, std::ios::binary | std::ios::ate);
  if (!file) {
    throw std::runtime_error("Failed to open file: " + file_path);
  }

  const auto file_size = file.tellg();
  if (file_size < 0) {
    throw std::runtime_error("Failed to get file size: " + file_path);
  }

  const size_t bytes = static_cast<size_t>(file_size);
  if (bytes % sizeof(T) != 0) {
    throw std::runtime_error("File size is not aligned with element size: " + file_path);
  }

  buffer.resize(bytes / sizeof(T));
  if (bytes == 0) {
    return;
  }

  file.seekg(0, std::ios::beg);
  file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(bytes));
  if (!file) {
    throw std::runtime_error("Failed to read file: " + file_path);
  }
}

}  // namespace qnnx
