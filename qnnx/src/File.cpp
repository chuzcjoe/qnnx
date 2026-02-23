#include "File.h"

namespace qnnx {

bool CheckFileExists(const std::string& filename) { return std::filesystem::exists(filename); }

bool CreateDirectory(const std::string& dir_path) {
  return std::filesystem::create_directory(dir_path);
}

#ifdef __hexagon__
size_t strnlen(const char* s, size_t n) {
  size_t i;
  for (i = 0; i < n && s[i] != '\0'; i++) continue;
  return i;
}
#endif

char* Strndup(const char* source, size_t maxlen) {
#ifdef __hexagon__
  size_t length = strnlen(source, maxlen);

  char* destination = (char*)malloc((length + 1) * sizeof(char));
  if (destination == nullptr) return nullptr;
  // copy length bytes to destination and leave destination[length] to be
  // null terminator
  strncpy(destination, source, length);
  destination[length] = '\0';
  return destination;
#else
  return strndup(source, maxlen);
#endif
}

size_t Memscpy(void* dst, size_t dst_size, const void* src, size_t copy_size) {
  if (!dst || !src || !dst_size || !copy_size) return 0;

  size_t min_size = dst_size < copy_size ? dst_size : copy_size;
  memcpy(dst, src, min_size);
  return min_size;
}

}  // namespace qnnx
