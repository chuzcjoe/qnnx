//==============================================================================
//
//  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
//  All rights reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.s
//
//==============================================================================

/**
 *  @file
 *  @brief QNN DSP component Context API.
 *
 *         The interfaces in this file work with the top level QNN
 *         API and supplements QnnContext.h for DSP backend
 */

#ifndef QNN_DSP_CONTEXT_H
#define QNN_DSP_CONTEXT_H

#include "QnnContext.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  QNN_DSP_CONTEXT_CONFIG_OPTION_FILE_READ_MEMORY_BUDGET = 10,
  QNN_DSP_CONTEXT_CONFIG_OPTION_UNKNOWN = 0x7fffffff
} QnnDspContext_ConfigOption_t;

typedef struct QnnDspContext_CustomConfig {
  QnnDspContext_ConfigOption_t option;
  union UNNAMED {
    // - Init time may be impacted depending the value set below
    // - Value should be greater than 0 and less than or equal to the file size
    //    - If set to 0, the feature is not utilized
    //    - If set to greater than file size, min(fileSize, fileReadMemoryBudgetInMb) is used
    // - As an example, if value 2 is passed, it would translate to (2 * 1024 * 1024) bytes
    uint32_t fileReadMemoryBudgetInMb;
  };
} QnnDspContext_CustomConfig_t;

// clang-format off
/// QnnDspContext_CustomConfig_t initializer macro
#define QNN_DSP_CONTEXT_CUSTOM_CONFIG_INIT                                  \
  {                                                                         \
    QNN_DSP_CONTEXT_CONFIG_OPTION_UNKNOWN,                 /*option*/       \
    0                                         /*fileReadMemoryBudgetInMb*/  \
  }

// clang-format on
#ifdef __cplusplus
}  // extern "C"
#endif

#endif
