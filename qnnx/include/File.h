#pragma once

#include <filesystem>

namespace qnnx {

bool CheckFileExists(const std::string& filename);

bool CreateDirectory(const std::string& dir_path);

}  // namespace qnnx