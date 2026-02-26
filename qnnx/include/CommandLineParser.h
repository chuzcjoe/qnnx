#pragma once

#include <string>

struct CommandLineParser {
  std::string arch;
  std::string backend;
  std::string model;
  int in_width;
  int in_height;
  int in_channels;
  int out_width;
  int out_height;
  int out_channels;
  std::string test_data;
  std::string output_dir;

  bool Parse(int argc, char** argv, std::string& error);
};
