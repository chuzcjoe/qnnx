#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "CommandLineParser.h"

int main(int argc, char** argv) {
  auto parser = std::make_unique<CommandLineParser>();
  std::string error;
  if (!parser->Parse(argc, argv, error)) {
    throw std::runtime_error("failed to parse command line options");
    return 1;
  }

  std::cout << "backend: " << parser->backend << '\n';
  std::cout << "model: " << parser->model << '\n';
  std::cout << "input_list: " << parser->input_list << '\n';
  std::cout << "output_dir: " << parser->output_dir << '\n';

  return 0;
}
