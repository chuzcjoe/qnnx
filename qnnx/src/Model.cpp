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
  auto build_id = GetBackendBuildId();
  QNNX_INFO("Backend build ID: {%s}", build_id.c_str());

  Assert(Initialize(), "failed to create folder and init log");
}

void Model::PopulateTensors() {
  // Populate tensors code here
}

void Model::Run() {
  // Run inference code here
}

QNNResults Model::Initialize() {
  if (dump_output_ && !CheckFileExists(output_path_)) {
    bool created = CreateDirectory(output_path_);
    if (created) {
      QNNX_INFO("Output directory created at %s", output_path_.c_str());
    } else {
      QNNX_ERROR("Failed to create output directory at %s", output_path_.c_str());
    }
  }

  return QNNResults::SUCCESS;
}

std::string Model::GetBackendBuildId() {
  char* backend_build_id = nullptr;
  if (QNN_SUCCESS !=
      function_pointers_.qnnInterface.backendGetBuildId((const char**)&backend_build_id)) {
    QNNX_ERROR("Unable to get build Id from the backend.");
  }
  return (backend_build_id == nullptr ? std::string("") : std::string(backend_build_id));
}

void Model::Assert(QNNResults result, const std::string& error_message) {
  if (result != QNNResults::SUCCESS) {
    throw std::runtime_error(error_message);
  }
}

}  // namespace qnnx