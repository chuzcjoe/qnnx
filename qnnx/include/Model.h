#pragma once

#include <chrono>
#include <string>

#include "Commons.h"
#include "File.h"
#include "Log.h"
#include "QnnxLog.h"
#include "Tensor.h"

namespace qnnx {

class Model {
 public:
  Model(ARCH arch, QnnFunctionPointers function_pointers, void* backend_handle,
        const std::string input_list_path, const std::string output_path,
        OutputDataType output_data_type = OutputDataType::FLOAT_ONLY,
        InputDataType input_data_type = InputDataType::FLOAT, const bool debug = false,
        const int num_inference = 1, const bool dump_output = false);

  ~Model();

  void Init();
  void PopulateTensors();
  void Run();

 private:
  QNNResults Initialize();

  QNNResults InitializeBackend();

  QNNResults IsDevicePropertySupported();

  QNNResults CreateDevice();

  QNNResults InitializeProfiling();

  QNNResults RegisterOpPackages();

  QNNResults CreateContext();

  QNNResults ComposeGraphs();

  QNNResults FinalizeGraphs();

  QNNResults ExecuteGraphs();

  QNNResults FreeContext();

  QNNResults TerminateBackend();

  std::string GetBackendBuildId();

  QNNResults IsFinalizeDeserializedGraphSupported();

  QNNResults FreeDevice();

  QNNResults VerifyFailReturnStatus(Qnn_ErrorHandle_t errCode);

  static void Assert(QNNResults result, const std::string& error_message);

  uint64_t GetTimeStampInUs() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
  }

 private:
  // commons
  ARCH arch_;
  QnnFunctionPointers function_pointers_;
  void* backend_handle_;
  std::string input_list_path_;
  std::string output_path_;
  OutputDataType output_data_type_;
  InputDataType input_data_type_;
  bool debug_;
  int num_inference_;
  bool dump_output_;

  // Log related
  std::shared_ptr<QnnxLog> qnnx_logger_ = nullptr;
  Qnn_LogHandle_t log_handle_ = nullptr;

  // backend related
  QnnBackend_Config_t** backend_config_ = nullptr;
  bool backend_initialized_ = false;

  // device related
  Qnn_DeviceHandle_t device_handle_ = nullptr;

  // profiling related
  ProfilingLevel profiling_level_ = ProfilingLevel::OFF;
  Qnn_ProfileHandle_t profile_backend_handle_ = nullptr;

  // context related
  Qnn_ContextHandle_t context_ = nullptr;
  QnnContext_Config_t** context_config_ = nullptr;
  bool context_created_ = false;

  // graph related
  GraphInfo_t** graphs_info_ = nullptr;
  uint32_t graphs_count_ = 0;
  GraphConfigInfo_t** graph_configs_info_ = nullptr;
  uint32_t graph_configs_info_count_ = 0;

  // others
  QnnSystemProfile_SerializationTargetHandle_t serialization_target_handle_ = nullptr;
  std::string save_binary_name_;
};

}  // namespace qnnx
