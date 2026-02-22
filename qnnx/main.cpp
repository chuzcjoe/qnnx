#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "CommandLineParser.h"
#include "Commons.h"
#include "Loader.h"
#include "Model.h"

static void* sg_backend_handle = nullptr;
static void* sg_model_handle = nullptr;

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

  // Load backend and model libraries
  qnnx::QnnFunctionPointers qnn_function_pointers;
  qnnx::QNNResults result = qnnx::GetQNNFunctionPointer(
      parser->backend, parser->model, &qnn_function_pointers, &sg_backend_handle,
      parser->model.empty() ? false : true, &sg_model_handle);
  if (result != qnnx::QNNResults::SUCCESS) {
    throw std::runtime_error("failed to load QNN libraries");
  }

  auto model = std::make_unique<qnnx::Model>(qnn_function_pointers, sg_backend_handle,
                                             parser->input_list, parser->output_dir);

  model->Init();

  return 0;
}
