#pragma once

#include <memory>
#include <queue>

#include "QnnBackend.h"
#include "QnnCommon.h"
#include "QnnContext.h"
#include "QnnGraph.h"
#include "QnnProperty.h"



namespace qnnx {

enum class OutputDataType { FLOAT_ONLY, NATIVE_ONLY, FLOAT_AND_NATIVE, INVALID };
enum class InputDataType { FLOAT, NATIVE, INVALID };

} // namespace qnnx