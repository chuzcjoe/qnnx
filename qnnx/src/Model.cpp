#include "Model.h"

namespace qnnx {

Model::Model(ARCH arch, QnnFunctionPointers function_pointers, void* backend_handle,
             const std::string input_list_path, const std::string output_path,
             OutputDataType output_data_type, InputDataType input_data_type, const bool debug,
             const int num_inference, const bool dump_output)
    : arch_(arch),
      function_pointers_(function_pointers),
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
  Assert(InitializeBackend(), "failed to initialize backend");
  Assert(IsDevicePropertySupported(), "device property is not supported by backend");
  Assert(CreateDevice(), "failed to create device");
  Assert(InitializeProfiling(), "failed to initialize profiling");
  Assert(RegisterOpPackages(), "failed to register op packages");

  switch (arch_) {
    case ARCH::CPU:
    case ARCH::GPU:
      Assert(CreateContext(), "failed to create context");
      Assert(ComposeGraphs(), "failed to compose graphs");
      Assert(FinalizeGraphs(), "failed to finalize graphs");
      break;
    case ARCH::DSP:
      QNNX_ERROR("DSP backend is not supported yet");
      throw std::runtime_error("DSP backend is not supported yet");
    case ARCH::HTP:
      QNNX_ERROR("HTP backend is not supported yet");
      throw std::runtime_error("HTP backend is not supported yet");
    default:
      QNNX_ERROR("Unsupported architecture");
      throw std::runtime_error("Unsupported architecture");
  }
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

  // Initialize logger
  if (nullptr == qnnx_logger_) {
    qnnx_logger_ = std::make_shared<QnnxLog>(nullptr);
    auto log_callback = qnnx_logger_->GetLogCallback();
    auto log_level = qnnx_logger_->GetLogLevel();

    auto result = function_pointers_.qnnInterface.logCreate(log_callback, log_level, &log_handle_);
    if (QNN_SUCCESS != result) {
      QNNX_WARN("Unable to initialize logging in the backend.");
    } else {
      QNNX_INFO("Logging initialized in the backend. Callback: [%p], Log Level: [%d]",
                (void*)log_callback, log_level);
    }
  }
  return QNNResults::SUCCESS;
}

// Initialize backend
QNNResults Model::InitializeBackend() {
  auto result = function_pointers_.qnnInterface.backendCreate(
      log_handle_, (const QnnBackend_Config_t**)backend_config_, &backend_handle_);

  if (QNN_BACKEND_NO_ERROR != result) {
    QNNX_ERROR("Could not initialize backend due to error = %lu", result);
    return QNNResults::FAIL;
  }

  QNNX_INFO("Initialize backend success, returned status = %lu", result);
  backend_initialized_ = true;
  return QNNResults::SUCCESS;
}

// check property support
QNNResults Model::IsDevicePropertySupported() {
  if (nullptr != function_pointers_.qnnInterface.propertyHasCapability) {
    auto result = function_pointers_.qnnInterface.propertyHasCapability(QNN_PROPERTY_GROUP_DEVICE);
    if (QNN_PROPERTY_NOT_SUPPORTED == result) {
      QNNX_WARN("Device property is not supported");
    } else if (QNN_PROPERTY_ERROR_UNKNOWN_KEY == result) {
      QNNX_ERROR("Device property is not known to backend");
      return QNNResults::FAIL;
    }
  }
  return QNNResults::SUCCESS;
}

// create device
QNNResults Model::CreateDevice() {
  if (nullptr != function_pointers_.qnnInterface.deviceCreate) {
    auto result =
        function_pointers_.qnnInterface.deviceCreate(log_handle_, nullptr, &device_handle_);
    if (QNN_SUCCESS != result && QNN_DEVICE_ERROR_UNSUPPORTED_FEATURE != result) {
      QNNX_ERROR("Failed to create device");

      QNNResults return_result = QNNResults::FAIL;
      switch (result) {
        case QNN_COMMON_ERROR_SYSTEM_COMMUNICATION:
          return_result = QNNResults::FAIL_SYSTEM_COMMUNICATION_ERROR;
          break;
        case QNN_COMMON_ERROR_SYSTEM:
          return_result = QNNResults::FAIL_SYSTEM_ERROR;
          break;
        case QNN_COMMON_ERROR_NOT_SUPPORTED:
          return_result = QNNResults::FAIL_FEATURE_UNSUPPORTED;
          break;
        default:
          break;
      }
      return return_result;
    }
  }
  return QNNResults::SUCCESS;
}

// TODO: Implement profiling initialization when profiling is enabled
QNNResults Model::InitializeProfiling() {
  if (ProfilingLevel::OFF == profiling_level_) {
    return QNNResults::SUCCESS;
  }
  return QNNResults::FAIL;
}

// TODO: Implement op package registration in the future
QNNResults Model::RegisterOpPackages() {
  // Register op packages code here
  return QNNResults::SUCCESS;
}

// create cpu/gpu context
QNNResults Model::CreateContext() {
  if (QNN_CONTEXT_NO_ERROR != function_pointers_.qnnInterface.contextCreate(
                                  backend_handle_, device_handle_,
                                  (const QnnContext_Config_t**)context_config_, &context_)) {
    QNNX_ERROR("Could not create context");
    return QNNResults::FAIL;
  }
  context_created_ = true;
  return QNNResults::SUCCESS;
}

// compose graphs, default is to use model.so
QNNResults Model::ComposeGraphs() {
  auto result = QNNResults::SUCCESS;

  // compose with QNN's model.so
  QNNX_INFO("Using model.so for graph composition");
  if (MODEL_NO_ERROR != function_pointers_.composeGraphsFnHandle(
                            backend_handle_, function_pointers_.qnnInterface, context_,
                            (const GraphConfigInfo_t**)graph_configs_info_,
                            graph_configs_info_count_, &graphs_info_, &graphs_count_, debug_,
                            qnnx_logger_->GetLogCallback(), qnnx_logger_->GetLogLevel())) {
    QNNX_ERROR("Failed in composeGraphs()");
    result = QNNResults::FAIL;
  }
  return result;
}

QNNResults Model::FinalizeGraphs() {
  for (size_t graph_idx = 0; graph_idx < graphs_count_; ++graph_idx) {
    // Profile this API call
    QnnSystemProfile_ProfileData_t profile_data = QNN_SYSTEM_PROFILE_DATA_INIT;
    if (ProfilingLevel::OFF != profiling_level_ && serialization_target_handle_ != nullptr) {
      profile_data.version = QNN_SYSTEM_PROFILE_DATA_VERSION_1;
      profile_data.v1.header.methodType = QNN_SYSTEM_PROFILE_METHOD_TYPE_BACKEND_FINALIZE;
      profile_data.v1.header.startTime = GetTimeStampInUs();
      profile_data.v1.header.graphName = (*graphs_info_)[graph_idx].graphName;
    }

    if (QNN_GRAPH_NO_ERROR !=
        function_pointers_.qnnInterface.graphFinalize((*graphs_info_)[graph_idx].graph,
                                                      profile_backend_handle_, nullptr)) {
      return QNNResults::FAIL;
    }

    // TODO: implement profiling stop time extraction
    // if (ProfilingLevel::OFF != profiling_level_) {
    //   if (serialization_target_handle_ != nullptr) {
    //     profile_data.v1.header.stopTime = GetTimeStampInUs();
    //     extractBackendProfilingInfo(profile_backend_handle_, &profile_data);
    //   } else {
    //     extractBackendProfilingInfo(profile_backend_handle_, nullptr);
    //   }
    // }
  }
  auto result = QNNResults::SUCCESS;
  if (!save_binary_name_.empty()) {
    QNNX_INFO("Before SaveBinary(): saving context and metadata.");
    // TODO: implement SaveBinary()
    // result = SaveBinary();
  } else {
    QNNX_DEBUG("save_binary_name_ is empty()");
  }
  return result;
}

// Get backend build ID
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