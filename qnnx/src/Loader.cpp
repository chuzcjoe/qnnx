#include "Loader.h"

namespace qnnx {

typedef Qnn_ErrorHandle_t (*QnnInterfaceGetProvidersFn_t)(const QnnInterface_t*** providerList,
                                                          uint32_t* numProviders);

typedef Qnn_ErrorHandle_t (*QnnSystemInterfaceGetProvidersFn_t)(
    const QnnSystemInterface_t*** providerList, uint32_t* numProviders);

template <class T>
static inline T ResolveSymbol(void* lib_handle, const char* sym) {
  T ptr = (T)dlSym(lib_handle, sym);
  if (ptr == nullptr) {
    QNNX_ERROR("Unable to access symbol [%s]. dlError(): %s", sym, dlError());
  }
  return ptr;
}

void* dlOpen(const char* filename, int flags) {
  int real_flags = 0;

  if (flags & DL_NOW) {
    real_flags |= RTLD_NOW;
  }

  if (flags & DL_LOCAL) {
    real_flags |= RTLD_LOCAL;
  }

  if (flags & DL_GLOBAL) {
    real_flags |= RTLD_GLOBAL;
  }

  if (flags & DL_NOLOAD) {
#ifndef __hexagon__
    real_flags |= RTLD_NOLOAD;
#else
    return nullptr;
#endif
  }

  return dlopen(filename, real_flags);
}

void* dlSym(void* handle, const char* symbol) {
  if (handle == DL_DEFAULT) {
    return dlsym(RTLD_DEFAULT, symbol);
  }

  return dlsym(handle, symbol);
}

int dlClose(void* handle) {
  if (!handle) {
    return 0;
  }

  return dlclose(handle);
}

char* dlError() {
  return dlerror();
}

QNNResults GetQNNFunctionPointer(std::string backend_path, std::string model_path,
                                 QnnFunctionPointers* function_pointers, void** backend_handle,
                                 bool load_model_lib, void** model_handle) {
  void* lib_backend_handle = dlOpen(backend_path.c_str(), DL_NOW | DL_GLOBAL);
  if (nullptr == lib_backend_handle) {
    QNNX_ERROR("Unable to load backend. dlError(): %s", dlError());
    return QNNResults::FAIL_LOAD_BACKEND;
  }

  if (nullptr != backend_handle) {
    *backend_handle = lib_backend_handle;
  }

  // Get QNN Interface
  QnnInterfaceGetProvidersFn_t get_Interface_providers{nullptr};
  get_Interface_providers =
      ResolveSymbol<QnnInterfaceGetProvidersFn_t>(lib_backend_handle, "QnnInterface_getProviders");
  if (nullptr == get_Interface_providers) {
    return QNNResults::FAIL_SYM_FUNCTION;
  }

  QnnInterface_t** interface_providers{nullptr};
  uint32_t num_providers{0};
  if (QNN_SUCCESS !=
      get_Interface_providers((const QnnInterface_t***)&interface_providers, &num_providers)) {
    QNNX_ERROR("Failed to get interface providers.");
    return QNNResults::FAIL_GET_INTERFACE_PROVIDERS;
  }
  if (nullptr == interface_providers) {
    QNNX_ERROR("Failed to get interface providers: null interface providers received.");
    return QNNResults::FAIL_GET_INTERFACE_PROVIDERS;
  }
  if (0 == num_providers) {
    QNNX_ERROR("Failed to get interface providers: 0 interface providers.");
    return QNNResults::FAIL_GET_INTERFACE_PROVIDERS;
  }

  bool found_valid_interface{false};
  for (size_t p_idx = 0; p_idx < num_providers; p_idx++) {
    if (QNN_API_VERSION_MAJOR == interface_providers[p_idx]->apiVersion.coreApiVersion.major &&
        QNN_API_VERSION_MINOR <= interface_providers[p_idx]->apiVersion.coreApiVersion.minor) {
      found_valid_interface = true;
      function_pointers->qnnInterface = interface_providers[p_idx]->QNN_INTERFACE_VER_NAME;
      function_pointers->qnnInterfaceHandle = *interface_providers[p_idx];
      break;
    }
  }
  if (!found_valid_interface) {
    QNNX_ERROR("Unable to find a valid interface.");
    lib_backend_handle = nullptr;
    return QNNResults::FAIL_GET_INTERFACE_PROVIDERS;
  }

  if (true == load_model_lib) {
    QNNX_INFO("Loading model shared library ([model].so)");
    void* lib_model_handle = dlOpen(model_path.c_str(), DL_NOW | DL_LOCAL);
    if (nullptr == lib_model_handle) {
      QNNX_ERROR("Unable to load model. dlError(): %s", dlError());
      return QNNResults::FAIL_LOAD_MODEL;
    }
    if (nullptr != model_handle) {
      *model_handle = lib_model_handle;
    }

    std::string model_prepare_func = "QnnModel_composeGraphs";
    function_pointers->composeGraphsFnHandle =
        ResolveSymbol<ComposeGraphsFnHandleType_t>(lib_model_handle,
                                                               model_prepare_func.c_str());
    if (nullptr == function_pointers->composeGraphsFnHandle) {
      return QNNResults::FAIL_SYM_FUNCTION;
    }

    std::string model_free_func = "QnnModel_freeGraphsInfo";
    function_pointers->freeGraphInfoFnHandle =
        ResolveSymbol<FreeGraphInfoFnHandleType_t>(lib_model_handle, model_free_func.c_str());
    if (nullptr == function_pointers->freeGraphInfoFnHandle) {
      return QNNResults::FAIL_SYM_FUNCTION;
    }
  } else {
    QNNX_INFO("Model wasn't loaded from a shared library.");
  }
  return QNNResults::SUCCESS;
}

}  // namespace qnnx