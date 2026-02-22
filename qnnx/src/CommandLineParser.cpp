#include "CommandLineParser.h"

#include <cstdint>
#include <string_view>

namespace {

enum class OptionKind : unsigned char {
  kArch,
  kBackend,
  kModel,
  kInputList,
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
    case HashName("input_list"):
      return name == "input_list" ? OptionKind::kInputList : OptionKind::kUnknown;
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

    std::string* out_value = nullptr;
    const char* option_name = nullptr;

    switch (ParseOption(option)) {
      case OptionKind::kArch:
        out_value = &arch;
        option_name = "--arch";
        break;
      case OptionKind::kBackend:
        out_value = &backend;
        option_name = "--backend";
        break;
      case OptionKind::kModel:
        out_value = &model;
        option_name = "--model";
        break;
      case OptionKind::kInputList:
        out_value = &input_list;
        option_name = "--input_list";
        break;
      case OptionKind::kOutputDir:
        out_value = &output_dir;
        option_name = "--output_dir";
        break;
      case OptionKind::kUnknown:
        error = "Unknown option: ";
        error += arg;
        return false;
    }

    if (has_inline_value) {
      out_value->assign(inline_value.data(), inline_value.size());
      continue;
    }

    if (i + 1 >= argc) {
      error = "Missing value for ";
      error += option_name;
      return false;
    }

    *out_value = argv[++i];
  }

  return true;
}
