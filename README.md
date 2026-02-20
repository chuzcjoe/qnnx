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
```
export QNN_SDK_ROOT="<qnn-sdk-path>/qairt/2.43.0.260128"
```