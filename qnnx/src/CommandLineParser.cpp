#include "CommandLineParser.h"

#include <charconv>
#include <cstdint>
#include <string_view>

namespace {

enum class OptionKind : unsigned char {
  kArch,
  kBackend,
  kModel,
  kInWidth,
  kInHeight,
  kInChannels,
  kOutWidth,
  kOutHeight,
  kOutChannels,
  kTestData,
  kOutputDir,
  kUnknown,
};

constexpr std::uint32_t HashName(std::string_view text) {
  std::uint32_t hash = 2166136261u;
  for (const char c : text) {
    hash ^= static_cast<unsigned char>(c);
    hash *= 16777619u;
  }
  return hash;
}

OptionKind ParseOption(std::string_view option) {
  if (option.size() <= 2 || option[0] != '-' || option[1] != '-') {
    return OptionKind::kUnknown;
  }

  const std::string_view name = option.substr(2);

  switch (HashName(name)) {
    case HashName("arch"):
      return name == "arch" ? OptionKind::kArch : OptionKind::kUnknown;
    case HashName("backend"):
      return name == "backend" ? OptionKind::kBackend : OptionKind::kUnknown;
    case HashName("model"):
      return name == "model" ? OptionKind::kModel : OptionKind::kUnknown;
    case HashName("in_width"):
      return name == "in_width" ? OptionKind::kInWidth : OptionKind::kUnknown;
    case HashName("in_height"):
      return name == "in_height" ? OptionKind::kInHeight : OptionKind::kUnknown;
    case HashName("in_channels"):
      return name == "in_channels" ? OptionKind::kInChannels : OptionKind::kUnknown;
    case HashName("out_width"):
      return name == "out_width" ? OptionKind::kOutWidth : OptionKind::kUnknown;
    case HashName("out_height"):
      return name == "out_height" ? OptionKind::kOutHeight : OptionKind::kUnknown;
    case HashName("out_channels"):
      return name == "out_channels" ? OptionKind::kOutChannels : OptionKind::kUnknown;
    case HashName("test_data"):
      return name == "test_data" ? OptionKind::kTestData : OptionKind::kUnknown;
    case HashName("output_dir"):
      return name == "output_dir" ? OptionKind::kOutputDir : OptionKind::kUnknown;
    default:
      return OptionKind::kUnknown;
  }
}

}  // namespace

bool CommandLineParser::Parse(int argc, char** argv, std::string& error) {
  for (int i = 1; i < argc; ++i) {
    const std::string_view arg(argv[i]);
    std::string_view option = arg;
    std::string_view inline_value;
    bool has_inline_value = false;

    const std::size_t equals_index = arg.find('=');
    if (equals_index != std::string_view::npos) {
      option = arg.substr(0, equals_index);
      inline_value = arg.substr(equals_index + 1);
      has_inline_value = true;
    }

    std::string* out_string = nullptr;
    int* out_int = nullptr;
    const char* option_name = nullptr;

    switch (ParseOption(option)) {
      case OptionKind::kArch:
        out_string = &arch;
        option_name = "--arch";
        break;
      case OptionKind::kBackend:
        out_string = &backend;
        option_name = "--backend";
        break;
      case OptionKind::kModel:
        out_string = &model;
        option_name = "--model";
        break;
      case OptionKind::kInWidth:
        out_int = &in_width;
        option_name = "--in_width";
        break;
      case OptionKind::kInHeight:
        out_int = &in_height;
        option_name = "--in_height";
        break;
      case OptionKind::kInChannels:
        out_int = &in_channels;
        option_name = "--in_channels";
        break;
      case OptionKind::kOutWidth:
        out_int = &out_width;
        option_name = "--out_width";
        break;
      case OptionKind::kOutHeight:
        out_int = &out_height;
        option_name = "--out_height";
        break;
      case OptionKind::kOutChannels:
        out_int = &out_channels;
        option_name = "--out_channels";
        break;
      case OptionKind::kTestData:
        out_string = &test_data;
        option_name = "--test_data";
        break;
      case OptionKind::kOutputDir:
        out_string = &output_dir;
        option_name = "--output_dir";
        break;
      case OptionKind::kUnknown:
        error = "Unknown option: ";
        error += arg;
        return false;
    }

    std::string_view value;
    if (has_inline_value) {
      value = inline_value;
    } else {
      if (i + 1 >= argc) {
        error = "Missing value for ";
        error += option_name;
        return false;
      }
      value = argv[++i];
    }

    if (out_string != nullptr) {
      out_string->assign(value.data(), value.size());
      continue;
    }

    int parsed = 0;
    const char* begin = value.data();
    const char* end = begin + value.size();
    auto [ptr, ec] = std::from_chars(begin, end, parsed);
    if (ec != std::errc() || ptr != end) {
      error = "Invalid integer value for ";
      error += option_name;
      return false;
    }
    *out_int = parsed;
  }

  return true;
}
