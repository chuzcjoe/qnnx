#pragma once

#include "QnnCommon.h"
#include "QnnInterface.h"
#include "System/QnnSystemInterface.h"

namespace qnnx {

enum class ARCH { CPU, GPU, DSP, HTP };

enum class QNNResults {
  SUCCESS,
  FAIL,
  FAIL_LOAD_BACKEND,
  FAIL_LOAD_MODEL,
  FAIL_SYM_FUNCTION,
  FAIL_GET_INTERFACE_PROVIDERS,
  FAIL_LOAD_SYSTEM_LIB,
  FAIL_SYSTEM_COMMUNICATION_ERROR,
  FAIL_SYSTEM_ERROR,
  FAIL_FEATURE_UNSUPPORTED
};

enum class ProfilingLevel { OFF, BASIC, DETAILED, INVALID };

typedef enum ModelError {
  MODEL_NO_ERROR = 0,
  MODEL_TENSOR_ERROR = 1,
  MODEL_PARAMS_ERROR = 2,
  MODEL_NODES_ERROR = 3,
  MODEL_GRAPH_ERROR = 4,
  MODEL_CONTEXT_ERROR = 5,
  MODEL_GENERATION_ERROR = 6,
  MODEL_SETUP_ERROR = 7,
  MODEL_INVALID_ARGUMENT_ERROR = 8,
  MODEL_FILE_ERROR = 9,
  MODEL_MEMORY_ALLOCATE_ERROR = 10,
  // Value selected to ensure 32 bits.
  MODEL_UNKNOWN_ERROR = 0x7FFFFFFF
} ModelError_t;

typedef struct GraphInfo {
  Qnn_GraphHandle_t graph;
  char *graphName;
  Qnn_Tensor_t *inputTensors;
  uint32_t numInputTensors;
  Qnn_Tensor_t *outputTensors;
  uint32_t numOutputTensors;
} GraphInfo_t;
typedef GraphInfo_t *GraphInfoPtr_t;

typedef struct GraphConfigInfo {
  char *graphName;
  const QnnGraph_Config_t **graphConfigs;
} GraphConfigInfo_t;

typedef ModelError_t (*ComposeGraphsFnHandleType_t)(Qnn_BackendHandle_t, QNN_INTERFACE_VER_TYPE,
                                                    Qnn_ContextHandle_t, const GraphConfigInfo_t **,
                                                    const uint32_t, GraphInfo_t ***, uint32_t *,
                                                    bool, QnnLog_Callback_t, QnnLog_Level_t);

typedef ModelError_t (*FreeGraphInfoFnHandleType_t)(GraphInfo_t ***, uint32_t);

typedef struct QnnFunctionPointers {
  ComposeGraphsFnHandleType_t composeGraphsFnHandle;
  FreeGraphInfoFnHandleType_t freeGraphInfoFnHandle;
  QNN_INTERFACE_VER_TYPE qnnInterface;
  QNN_SYSTEM_INTERFACE_VER_TYPE qnnSystemInterface;
  QnnInterface_t qnnInterfaceHandle;
  QnnSystemInterface_t qnnSystemInterfaceHandle;
} QnnFunctionPointers;

}  // namespace qnnx
