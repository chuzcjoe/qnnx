#pragma once

#include <string>

struct CommandLineParser {
  std::string arch;
  std::string backend;
  std::string model;
  std::string input_list;
  std::string output_dir;

  bool Parse(int argc, char** argv, std::string& error);
};
