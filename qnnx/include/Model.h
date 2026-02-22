#pragma once

#include <string>

#include "Commons.h"
#include "File.h"
#include "Log.h"
#include "QnnxLog.h"
#include "Tensor.h"

namespace qnnx {

class Model {
 public:
  Model(QnnFunctionPointers function_pointers, void* backend_handle,
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

 private:
  // commons
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
};

}  // namespace qnnx
