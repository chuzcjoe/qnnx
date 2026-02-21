#pragma once

#include <dlfcn.h>
#include <stdlib.h>

#include <string>

#include "Commons.h"
#include "Log.h"

namespace qnnx {

#define DL_DEFAULT (void *)(0x4)

enum { 
    DL_NOW = 0x0001, 
    DL_LOCAL = 0x0002, 
    DL_GLOBAL = 0x0004, 
    DL_NOLOAD = 0x0008 
};

void* dlOpen(const char* filename, int flags);

void* dlSym(void* handle, const char* symbol);

int dlClose(void* handle);

char* dlError();

QNNResults GetQNNFunctionPointer(std::string backend_path, std::string model_path,
                                 QnnFunctionPointers* function_pointers, void** backend_handle,
                                 bool load_model_lib, void** model_handle);

QNNResults GetQNNSystemPointer();

}  // namespace qnnx