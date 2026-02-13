#!/bin/bash
set -ex

echo "Usage: $0 <backend: cpu|gpu|htp>"

backend=$1

rm -rf build
mkdir -p build
cd build

device_path=/data/local/tmp/qnnx

cmake_options+=(-DCMAKE_BUILD_TYPE=Debug
                -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake
                -DANDROID_ABI=$target
                -DANDROID_PLATFORM=android-34)

cmake "${cmake_options[@]}" ../
make -j10

cd ..


adb shell "rm -rf ${device_path}"
adb shell "mkdir -p ${device_path}"
adb shell "mkdir -p ${device_path}/output"

adb push ./build/sample_app/qnn-sample-app ${device_path}
adb push ./model_tmp/fp32/${QNN_TARGET_ARCH}/libsegnet_fp32.so ${device_path}
adb push ./model_zoo/python/unet/test.raw ${device_path}
adb push ./model_tmp/input_list.txt ${device_path}


if [ "$backend" = "cpu" ]; then
    adb push ${QNN_SDK_ROOT}/lib/${QNN_TARGET_ARCH}/libQnnCpu.so ${device_path}
    adb push ${QNN_SDK_ROOT}/lib/${QNN_TARGET_ARCH}/libQnnSystem.so ${device_path}

    adb shell "chmod +x ${device_path}/qnn-sample-app"
    adb shell "export LD_LIBRARY_PATH=${device_path} && export ADSP_LIBRARY_PATH=${device_path} && ${device_path}/qnn-sample-app --backend ${device_path}/libQnnCpu.so --model ${device_path}/libsegnet_fp32.so --input_list ${device_path}/input_list.txt --output_dir ${device_path}/output"
elif [ "$backend" = "gpu" ]; then
    adb push ${QNN_SDK_ROOT}/lib/${QNN_TARGET_ARCH}/libQnnGpu.so ${device_path}
    adb push ${QNN_SDK_ROOT}/lib/${QNN_TARGET_ARCH}/libQnnSystem.so ${device_path}

    adb shell "chmod +x ${device_path}/qnn-sample-app"
    adb shell "export LD_LIBRARY_PATH=${device_path} && \
                export ADSP_LIBRARY_PATH=${device_path} &&  \
                ${device_path}/qnn-sample-app --backend ${device_path}/libQnnGpu.so --model ${device_path}/libsegnet_fp32.so --input_list ${device_path}/input_list.txt --output_dir ${device_path}/output"
elif [ "$backend" = "htp" ]; then
    echo "run htp"
else
    echo "Unsupported backend: $backend"
    exit 1
fi
