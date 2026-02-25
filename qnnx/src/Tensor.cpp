#include "Tensor.h"

#include <cstring>

#include "File.h"
#include "Types.h"

namespace qnnx {

QNNResults Tensor::SetupInputAndOutputTensors(Qnn_Tensor_t** inputs, Qnn_Tensor_t** outputs,
                                              GraphInfo_t graph_info) {
  auto result = QNNResults::SUCCESS;

  // Setup input tensors
  if (QNNResults::SUCCESS !=
      SetupTensors(inputs, graph_info.numInputTensors, (graph_info.inputTensors))) {
    QNNX_ERROR("Failure in setting up input tensors");
    result = QNNResults::FAIL;
  }

  // Setup output tensors
  if (QNNResults::SUCCESS !=
      SetupTensors(outputs, graph_info.numOutputTensors, (graph_info.outputTensors))) {
    QNNX_ERROR("Failure in setting up output tensors");
    result = QNNResults::FAIL;
  }

  // Clean up resources
  if (QNNResults::SUCCESS != result) {
    QNNX_ERROR("Failure in SetupInputAndOutputTensors, cleaning up resources");
    if (nullptr != *inputs) {
      QNNX_DEBUG("cleaning up input tensors");
      TearDownTensors(*inputs, graph_info.numInputTensors);
      *inputs = nullptr;
    }
    if (nullptr != *outputs) {
      QNNX_DEBUG("cleaning up output tensors");
      TearDownTensors(*outputs, graph_info.numOutputTensors);
      *outputs = nullptr;
    }
    QNNX_ERROR("Failure in SetupInputAndOutputTensors, done cleaning up resources");
  }
  return result;
}

QNNResults Tensor::SetupTensors(Qnn_Tensor_t** tensors, uint32_t tensor_count,
                                Qnn_Tensor_t* tensors_info) {
  if (nullptr == tensors_info) {
    QNNX_ERROR("tensors_info is nullptr");
    return QNNResults::FAIL;
  }
  if (0 == tensor_count) {
    QNNX_INFO("tensor count is 0. Nothing to setup.");
    return QNNResults::SUCCESS;
  }

  auto result = QNNResults::SUCCESS;
  *tensors = (Qnn_Tensor_t*)calloc(1, tensor_count * sizeof(Qnn_Tensor_t));
  if (nullptr == *tensors) {
    QNNX_ERROR("mem alloc failed for *tensors");
    result = QNNResults::FAIL;
    return result;
  }

  for (size_t tensor_idx = 0; tensor_idx < tensor_count; ++tensor_idx) {
    Qnn_Tensor_t tensor_wrapper = tensors_info[tensor_idx];
    std::vector<size_t> dims;
    GetDimensions(dims, QNN_TENSOR_GET_DIMENSIONS(tensor_wrapper),
                  QNN_TENSOR_GET_RANK(tensor_wrapper));
    if (QNNResults::SUCCESS == result) {
      (*tensors)[tensor_idx] = QNN_TENSOR_INIT;
      result = (DeepCopyQnnTensorInfo(((*tensors) + tensor_idx), &tensor_wrapper) == true
                    ? QNNResults::SUCCESS
                    : QNNResults::FAIL);
    }

    if (QNNResults::SUCCESS == result) {
      QNNX_DEBUG("DeepCopyQnnTensorInfo successful");
      QNN_TENSOR_SET_MEM_TYPE(((*tensors) + tensor_idx), QNN_TENSORMEMTYPE_RAW);
    }

    // Setup client buffer
    Qnn_ClientBuffer_t client_buffer = QNN_CLIENT_BUFFER_INIT;
    result = AllocateBuffer(reinterpret_cast<uint8_t**>(&client_buffer.data), dims,
                            QNN_TENSOR_GET_DATA_TYPE((*tensors) + tensor_idx));
    if (QNNResults::SUCCESS != result) {
      QNNX_ERROR("failure in AllocateBuffer");
    }

    size_t length = 0;
    length = CalculateLength(dims, QNN_TENSOR_GET_DATA_TYPE((*tensors) + tensor_idx));
    if (length == 0) {
      QNNX_WARN("Calculated length is zero");
    }

    client_buffer.dataSize = length;
    QNN_TENSOR_SET_CLIENT_BUF(((*tensors) + tensor_idx), client_buffer);
    if (QNNResults::SUCCESS != result) {
      QNNX_ERROR("Failure in SetupTensors, cleaning up resources");
      if (nullptr != (QNN_TENSOR_GET_CLIENT_BUF((*tensors) + tensor_idx)).data) {
        free(QNN_TENSOR_GET_CLIENT_BUF((*tensors) + tensor_idx).data);
      }
      TearDownTensors(*tensors, tensor_idx);
      *tensors = nullptr;
      result = QNNResults::FAIL;
      QNNX_ERROR("Failure in SetupTensors, done cleaning up resources");
      return result;
    }
  }
  return result;
}

QNNResults Tensor::TearDownTensors(Qnn_Tensor_t* tensors, uint32_t tensor_count) {
  for (size_t tensor_idx = 0; tensor_idx < tensor_count; ++tensor_idx) {
    QNNX_DEBUG("freeing resources for tensor: %zu", tensor_idx);
    if (nullptr != QNN_TENSOR_GET_NAME(tensors[tensor_idx])) {
      QNNX_DEBUG("freeing tensor name");
      free(const_cast<char*>(QNN_TENSOR_GET_NAME(tensors[tensor_idx])));
      QNN_TENSOR_SET_NAME(tensors[tensor_idx], nullptr);
    }

    if (nullptr != QNN_TENSOR_GET_DIMENSIONS(tensors[tensor_idx])) {
      QNNX_DEBUG("freeing dimensions");
      free(QNN_TENSOR_GET_DIMENSIONS(tensors[tensor_idx]));
      QNN_TENSOR_SET_DIMENSIONS(tensors[tensor_idx], nullptr);
    }

    if (nullptr != QNN_TENSOR_GET_CLIENT_BUF(tensors[tensor_idx]).data) {
      QNNX_DEBUG("freeing clientBuf.data");
      free(QNN_TENSOR_GET_CLIENT_BUF(tensors[tensor_idx]).data);
    }
  }
  free(tensors);
  return QNNResults::SUCCESS;
}

QNNResults Tensor::GetDimensions(std::vector<size_t>& dims, uint32_t* tensor_dims, uint32_t rank) {
  if (nullptr == tensor_dims) {
    QNNX_ERROR("tensor_dims is nullptr");
    return QNNResults::FAIL;
  }
  if (rank == 0) {
    QNNX_ERROR("rank is 0");
    return QNNResults::FAIL;
  }

  dims.clear();
  for (uint32_t i = 0; i < rank; ++i) {
    dims.push_back(tensor_dims[i]);
  }
  return QNNResults::SUCCESS;
}

bool Tensor::DeepCopyQnnTensorInfo(Qnn_Tensor_t* dst, const Qnn_Tensor_t* src) {
  if (nullptr == dst || nullptr == src) {
    QNNX_ERROR("DeepCopyQnnTensorInfo: Received nullptr");
    return false;
  }

  // set tensor.version before using QNN_TENSOR_SET macros, as they require the version to be set
  // to correctly assign values
  dst->version = src->version;
  const char* tensor_name = QNN_TENSOR_GET_NAME(src);
  if (!tensor_name) {
    QNN_TENSOR_SET_NAME(dst, nullptr);
  } else {
    QNN_TENSOR_SET_NAME(dst, Strndup(tensor_name, strlen(tensor_name)));
  }
  QNN_TENSOR_SET_ID(dst, QNN_TENSOR_GET_ID(src));
  QNN_TENSOR_SET_TYPE(dst, QNN_TENSOR_GET_TYPE(src));
  QNN_TENSOR_SET_DATA_FORMAT(dst, QNN_TENSOR_GET_DATA_FORMAT(src));
  QNN_TENSOR_SET_DATA_TYPE(dst, QNN_TENSOR_GET_DATA_TYPE(src));
  Qnn_QuantizeParams_t q_params = QNN_QUANTIZE_PARAMS_INIT;
  q_params.encodingDefinition = QNN_TENSOR_GET_QUANT_PARAMS(src).encodingDefinition;
  q_params.quantizationEncoding = QNN_QUANTIZATION_ENCODING_UNDEFINED;
  if (QNN_TENSOR_GET_QUANT_PARAMS(src).quantizationEncoding ==
      QNN_QUANTIZATION_ENCODING_SCALE_OFFSET) {
    q_params.quantizationEncoding = QNN_TENSOR_GET_QUANT_PARAMS(src).quantizationEncoding;
    q_params.scaleOffsetEncoding = QNN_TENSOR_GET_QUANT_PARAMS(src).scaleOffsetEncoding;
  } else if (QNN_TENSOR_GET_QUANT_PARAMS(src).quantizationEncoding ==
             QNN_QUANTIZATION_ENCODING_AXIS_SCALE_OFFSET) {
    q_params.quantizationEncoding = QNN_TENSOR_GET_QUANT_PARAMS(src).quantizationEncoding;
    q_params.axisScaleOffsetEncoding.axis =
        QNN_TENSOR_GET_QUANT_PARAMS(src).axisScaleOffsetEncoding.axis;
    q_params.axisScaleOffsetEncoding.numScaleOffsets =
        QNN_TENSOR_GET_QUANT_PARAMS(src).axisScaleOffsetEncoding.numScaleOffsets;
    if (QNN_TENSOR_GET_QUANT_PARAMS(src).axisScaleOffsetEncoding.numScaleOffsets > 0) {
      q_params.axisScaleOffsetEncoding.scaleOffset = (Qnn_ScaleOffset_t*)malloc(
          QNN_TENSOR_GET_QUANT_PARAMS(src).axisScaleOffsetEncoding.numScaleOffsets *
          sizeof(Qnn_ScaleOffset_t));
      if (q_params.axisScaleOffsetEncoding.scaleOffset) {
        for (size_t idx = 0;
             idx < QNN_TENSOR_GET_QUANT_PARAMS(src).axisScaleOffsetEncoding.numScaleOffsets;
             ++idx) {
          q_params.axisScaleOffsetEncoding.scaleOffset[idx].scale =
              QNN_TENSOR_GET_QUANT_PARAMS(src).axisScaleOffsetEncoding.scaleOffset[idx].scale;
          q_params.axisScaleOffsetEncoding.scaleOffset[idx].offset =
              QNN_TENSOR_GET_QUANT_PARAMS(src).axisScaleOffsetEncoding.scaleOffset[idx].offset;
        }
      }
    }
  }
  QNN_TENSOR_SET_QUANT_PARAMS(dst, q_params);
  QNN_TENSOR_SET_RANK(dst, QNN_TENSOR_GET_RANK(src));
  QNN_TENSOR_SET_DIMENSIONS(dst, nullptr);
  if (QNN_TENSOR_GET_RANK(src) > 0) {
    QNN_TENSOR_SET_DIMENSIONS(dst, (uint32_t*)malloc(QNN_TENSOR_GET_RANK(src) * sizeof(uint32_t)));
    if (QNN_TENSOR_GET_DIMENSIONS(dst)) {
      Memscpy(QNN_TENSOR_GET_DIMENSIONS(dst), QNN_TENSOR_GET_RANK(src) * sizeof(uint32_t),
              QNN_TENSOR_GET_DIMENSIONS(src), QNN_TENSOR_GET_RANK(src) * sizeof(uint32_t));
    }
    if (QNN_TENSOR_GET_IS_DYNAMIC_DIMENSIONS(src)) {
      QNN_TENSOR_SET_IS_DYNAMIC_DIMENSIONS(
          dst, (uint8_t*)malloc(QNN_TENSOR_GET_RANK(src) * sizeof(uint8_t)));
      Memscpy(QNN_TENSOR_GET_IS_DYNAMIC_DIMENSIONS(dst), QNN_TENSOR_GET_RANK(src) * sizeof(uint8_t),
              QNN_TENSOR_GET_IS_DYNAMIC_DIMENSIONS(src),
              QNN_TENSOR_GET_RANK(src) * sizeof(uint8_t));
    }
  }
  QNN_TENSOR_SET_SPARSE_PARAMS(dst, QNN_TENSOR_GET_SPARSE_PARAMS(src));
  return true;
}

QNNResults Tensor::AllocateBuffer(uint8_t** buffer, std::vector<size_t> dims,
                                  Qnn_DataType_t data_type) {
  size_t element_count = CalculateElementCount(dims);
  auto result = QNNResults::SUCCESS;
  switch (data_type) {
    case QNN_DATATYPE_FLOAT_32:
      QNNX_DEBUG("allocating float buffer");
      result = AllocateBuffer<float>(reinterpret_cast<float**>(buffer), element_count);
      break;

    case QNN_DATATYPE_UINT_8:
    case QNN_DATATYPE_UFIXED_POINT_8:
      QNNX_DEBUG("allocating uint8_t buffer");
      result = AllocateBuffer<uint8_t>(reinterpret_cast<uint8_t**>(buffer), element_count);
      break;

    case QNN_DATATYPE_UINT_16:
    case QNN_DATATYPE_UFIXED_POINT_16:
      QNNX_DEBUG("allocating uint16_t buffer");
      result = AllocateBuffer<uint16_t>(reinterpret_cast<uint16_t**>(buffer), element_count);
      break;

    case QNN_DATATYPE_UINT_32:
      QNNX_DEBUG("allocating uint32_t buffer");
      result = AllocateBuffer<uint32_t>(reinterpret_cast<uint32_t**>(buffer), element_count);
      break;

    case QNN_DATATYPE_UINT_64:
      QNNX_DEBUG("allocating uint64_t buffer");
      result = AllocateBuffer<uint64_t>(reinterpret_cast<uint64_t**>(buffer), element_count);
      break;

    case QNN_DATATYPE_INT_8:
      QNNX_DEBUG("allocating int8_t buffer");
      result = AllocateBuffer<int8_t>(reinterpret_cast<int8_t**>(buffer), element_count);
      break;

    case QNN_DATATYPE_INT_16:
      QNNX_DEBUG("allocating int16_t buffer");
      result = AllocateBuffer<int16_t>(reinterpret_cast<int16_t**>(buffer), element_count);
      break;

    case QNN_DATATYPE_INT_32:
      QNNX_DEBUG("allocating int32_t buffer");
      result = AllocateBuffer<int32_t>(reinterpret_cast<int32_t**>(buffer), element_count);
      break;

    case QNN_DATATYPE_INT_64:
      QNNX_DEBUG("allocating int64_t buffer");
      result = AllocateBuffer<int64_t>(reinterpret_cast<int64_t**>(buffer), element_count);
      break;

    case QNN_DATATYPE_BOOL_8:
      QNNX_DEBUG("allocating bool buffer");
      result = AllocateBuffer<uint8_t>(reinterpret_cast<uint8_t**>(buffer), element_count);
      break;

    default:
      QNNX_ERROR("Datatype not supported yet!");
      result = QNNResults::FAIL;
      break;
  }
  return result;
}

QNNResults Tensor::FillInputTensors(const uint8_t** data, Qnn_Tensor_t* inputs,
                                    GraphInfo_t graph_info, InputDataType input_data_type) {
  auto result = QNNResults::SUCCESS;

  if (nullptr == inputs) {
    QNNX_ERROR("FillInputTensors failed, inputs is nullptr");
    return QNNResults::FAIL;
  }

  // The number of input tensors here means the number of different model inputs
  // for example, for denoise task, the model could have two inputs:
  // 1: a noisy image
  // 2: a mask to guide denoising
  // Most tasks have just one input
  const auto input_tensor_count = graph_info.numInputTensors;
  for (size_t i = 0; i < input_tensor_count; ++i) {
    result = FillInputTensor(data[i], &(inputs[i]), input_data_type);
    if (QNNResults::SUCCESS != result) {
      QNNX_ERROR("Failed to FillInputTensor for tensor index: %zu", i);
      return QNNResults::FAIL;
    }
  }

  return result;
}

QNNResults Tensor::FillInputTensor(const uint8_t* data, Qnn_Tensor_t* input,
                                   InputDataType input_data_type) {
  auto result = QNNResults::SUCCESS;

  if (nullptr == input || nullptr == data) {
    QNNX_ERROR("FillInputTensor failed, input or data is nullptr");
    return QNNResults::FAIL;
  }

  std::vector<size_t> dims;
  GetDimensions(dims, QNN_TENSOR_GET_DIMENSIONS(input), QNN_TENSOR_GET_RANK(input));

  const auto input_tensor_type = QNN_TENSOR_GET_DATA_TYPE(input);
  size_t length = CalculateLength(dims, input_tensor_type);

  QNNX_INFO("Filling input tensor, input tensor type %d", input_tensor_type);
  if (input_data_type == InputDataType::FLOAT && input_tensor_type != QNN_DATATYPE_FLOAT_32) {
    throw std::runtime_error(
        "Input data type is FLOAT but input tensor data type is not FLOAT_32, to be implemented");
  } else {
    auto client_buffer = static_cast<uint8_t*>(QNN_TENSOR_GET_CLIENT_BUF(input).data);
    std::memcpy(client_buffer, data, length);
  }

  return result;
}

}  // namespace qnnx
