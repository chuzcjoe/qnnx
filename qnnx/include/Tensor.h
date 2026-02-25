#pragma once

#include <map>
#include <memory>
#include <numeric>
#include <queue>

#include "Commons.h"
#include "Log.h"
#include "QnnBackend.h"
#include "QnnCommon.h"
#include "QnnContext.h"
#include "QnnGraph.h"
#include "QnnProperty.h"

namespace qnnx {

enum class OutputDataType { FLOAT_ONLY, NATIVE_ONLY, FLOAT_AND_NATIVE, INVALID };
enum class InputDataType { FLOAT, NATIVE, INVALID };

static const std::map<Qnn_DataType_t, size_t> kDataTypeToSizeMap = {
    {QNN_DATATYPE_INT_8, 1},           {QNN_DATATYPE_INT_16, 2},
    {QNN_DATATYPE_INT_32, 4},          {QNN_DATATYPE_INT_64, 8},
    {QNN_DATATYPE_UINT_8, 1},          {QNN_DATATYPE_UINT_16, 2},
    {QNN_DATATYPE_UINT_32, 4},         {QNN_DATATYPE_UINT_64, 8},
    {QNN_DATATYPE_FLOAT_16, 2},        {QNN_DATATYPE_BFLOAT_16, 2},
    {QNN_DATATYPE_FLOAT_32, 4},        {QNN_DATATYPE_FLOAT_64, 8},
    {QNN_DATATYPE_SFIXED_POINT_8, 1},  {QNN_DATATYPE_SFIXED_POINT_16, 2},
    {QNN_DATATYPE_SFIXED_POINT_32, 4}, {QNN_DATATYPE_UFIXED_POINT_8, 1},
    {QNN_DATATYPE_UFIXED_POINT_16, 2}, {QNN_DATATYPE_UFIXED_POINT_32, 4},
    {QNN_DATATYPE_BOOL_8, 1},
};

struct Tensor {
  Tensor() = default;

  QNNResults SetupInputAndOutputTensors(Qnn_Tensor_t** inputs, Qnn_Tensor_t** outputs,
                                        GraphInfo_t graph_info);

  QNNResults FillInputTensors(const uint8_t** data, Qnn_Tensor_t* inputs, GraphInfo_t graph_info,
                              InputDataType input_data_type);

 private:
  QNNResults SetupTensors(Qnn_Tensor_t** tensors, uint32_t tensor_count,
                          Qnn_Tensor_t* tensors_info);

  QNNResults TearDownTensors(Qnn_Tensor_t* tensors, uint32_t tensor_count);

  QNNResults GetDimensions(std::vector<size_t>& dims, uint32_t* tensor_dims, uint32_t rank);

  bool DeepCopyQnnTensorInfo(Qnn_Tensor_t* dst, const Qnn_Tensor_t* src);

  QNNResults AllocateBuffer(uint8_t** buffer, std::vector<size_t> dims, Qnn_DataType_t data_type);

  QNNResults FillInputTensor(const uint8_t* data, Qnn_Tensor_t* input,
                             InputDataType input_data_type);

  size_t CalculateElementCount(std::vector<size_t> dims) const {
    if (dims.size() == 0) return 0;
    return std::accumulate(dims.begin(), dims.end(), 1, std::multiplies<size_t>());
  }

  template <typename T>
  QNNResults AllocateBuffer(T** buffer, size_t& element_count) {
    QNNX_DEBUG("ElementCount: %zu, sizeof(T): %lu, total size: %zu", element_count, sizeof(T),
               element_count * sizeof(T));
    *buffer = (T*)malloc(element_count * sizeof(T));
    if (nullptr == *buffer) {
      QNNX_ERROR("mem alloc failed for *buffer");
      return QNNResults::FAIL;
    }
    return QNNResults::SUCCESS;
  }

  size_t CalculateLength(std::vector<size_t> dims, Qnn_DataType_t data_type) {
    if (dims.size() == 0) return 0;
    size_t length = 0;
    if (kDataTypeToSizeMap.find(data_type) != kDataTypeToSizeMap.end()) {
      length = kDataTypeToSizeMap.at(data_type);
    }
    length *= CalculateElementCount(dims);
    return length;
  }
};

}  // namespace qnnx