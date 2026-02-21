#include "Model.h"

namespace qnnx {

Model::Model(QnnFunctionPointers function_pointers, void* backend_handle,
             const std::string input_list_path, const std::string output_path,
             OutputDataType output_data_type, InputDataType input_data_type, const bool debug,
             const int num_inference, const bool dump_output)
    : function_pointers_(function_pointers),
      backend_handle_(backend_handle),
      input_list_path_(input_list_path),
      output_path_(output_path),
      output_data_type_(output_data_type),
      input_data_type_(input_data_type),
      debug_(debug),
      num_inference_(num_inference),
      dump_output_(dump_output) {}

Model::~Model() {}

void Model::Init() {
  // Initialization code here
}

void Model::PopulateTensors() {
  // Populate tensors code here
}

void Model::Run() {
  // Run inference code here
}

}  // namespace qnnx