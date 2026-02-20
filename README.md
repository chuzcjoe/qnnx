# QNNX — A lightweight C++ wrapper around Qualcomm AI Engine (QNN)

<!-- <p align="center">
  <img src="./imgs/qnnx_logo.png" alt="qnnx" width="60%"/>
</p> -->

# 1. Environment

1. Ubuntu 22.04 (x86-64 architecture) is required as the operating system.
2. Python 3.10 is required and has been verified to work correctly with this project.
3. QNN SDK is required, for this project, i am using 2.43.0.260128. It can be downloaded from [here](https://www.qualcomm.com/developer/software/qualcomm-ai-engine-direct-sdk)

After QNN SDK is downloaded and extracted, we need to configure the `bashrc.` or `.zshrc` file to export ANDROID NDK path.

```bash
export ANDROID_NDK_ROOT=<ndk-path>/android-ndk-r26c
export PATH="${ANDROID_NDK_ROOT}:${PATH}"
```

In `setup_environment.sh`, configure the `QNN_SDK_ROOT` to your local path
```bash
export QNN_SDK_ROOT="<qnn-sdk-path>/qairt/2.43.0.260128"
```

# 2. Convert Pytorch Model to QNN model

## 2.1 Full precision model

Full precision model can be used for CPU/GPU backend inference. Some pytorch examples are provided under `./python` folder. For example, to convert a FCN segmentation pytorch model to a full precision QNN model, we need to run:

```bash
./convert_to_qnn.sh ./python/fcn/fcn.pt 1 3 750 1000 cpu

# or

./convert_to_qnn.sh ./python/fcn/fcn.pt 1 3 750 1000 gpu
```

## 2.2 Quantized model

Quantized model is used for HTP backend inference. Quantized models have to be compatible with specific device HTP arch (e.g. v68, v73, v79 etc). So we need to specify which HTP arch we are going to use. The default is v73.

```bash
./convert_to_qnn.sh ./python/fcn/fcn.pt 1 3 750 1000 htp v73
```