#include "File.h"

namespace qnnx {

bool CheckFileExists(const std::string& filename) { return std::filesystem::exists(filename); }

bool CreateDirectory(const std::string& dir_path) {
  return std::filesystem::create_directory(dir_path);
}

}  // namespace qnnx
