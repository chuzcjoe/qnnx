#pragma once

#include <dlfcn.h>
#include <stdlib.h>

#include <string>

#include "QNNCommons.h"

namespace qnnx {

void dlOpen(const char* filename, int flags);

void dlSym(void* handle, const char* symbol);

void dlClose(void* handle);

char dlError();

QNNResults GetQNNFunctionPointer(std::string backend_path, std::string model_path,
                                 QnnFunctionPointers* function_pointer, void** backend_handle,
                                 bool load_model_lib, void** model_handle);

QNNResults GetQNNSystemPointer();

}  // namespace qnnx