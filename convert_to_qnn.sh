#!/usr/bin/env bash
set -e

echo "usage: $0 <path-to-pytorch-model> <batch num> <channels> <height> <width> <beackend> <htp version>"

MODEL_PATH=$1
BATCH_NUM=$2
CHANNELS=$3
HEIGHT=$4
WIDTH=$5
BACKEND=$6
HTP_VERSION=${7:-v73}

if [ "$BACKEND" = "cpu" ] || [ "$BACKEND" = "gpu" ]; then
    source ./setup_environment.sh aarch64-android
elif [ "$BACKEND" = "htp" ]; then
    source ./setup_environment.sh x86_64-linux-clang
else
    echo "Unsupported backend: $BACKEND"
    exit 1
fi

rm -rf model_tmp
mkdir -p model_tmp

echo "QNN_SDK_ROOT: ${QNN_SDK_ROOT}"

CURRENT_DIR=$PWD

if [ "$BACKEND" = "cpu" ] || [ "$BACKEND" = "gpu" ]; then
    echo "Converting Pytorch model to FP32 QNN model"

    TORCH_MODEL_NAME=$(basename "$MODEL_PATH" .pt)
    QNN_MODEL_NAME="${TORCH_MODEL_NAME}_fp32"

    QNN_MODEL_CPP_NAME="${QNN_MODEL_NAME}.cpp"
    QNN_MODEL_BIN_NAME="${QNN_MODEL_NAME}.bin"

    python3 "${QNN_SDK_ROOT}/bin/x86_64-linux-clang/qnn-pytorch-converter" \
    --input_network "${MODEL_PATH}" \
    --input_dim input $BATCH_NUM,$CHANNELS,$HEIGHT,$WIDTH \
    -o ./model_tmp/$QNN_MODEL_CPP_NAME

    python3 "${QNN_SDK_ROOT}/bin/x86_64-linux-clang/qnn-model-lib-generator" \
    -c ./model_tmp/${QNN_MODEL_CPP_NAME} \
    -b ./model_tmp/${QNN_MODEL_BIN_NAME} \
    -o ./model_tmp/fp32 \
    -t $QNN_TARGET_ARCH


elif [ "$BACKEND" = "htp" ]; then
    echo "Converting Pytorch model to Quantized QNN model"

    TORCH_MODEL_NAME=$(basename "$MODEL_PATH" .pt)
    QNN_MODEL_NAME="${TORCH_MODEL_NAME}_quant"

    QNN_MODEL_CPP_NAME="${QNN_MODEL_NAME}.cpp"
    QNN_MODEL_BIN_NAME="${QNN_MODEL_NAME}.bin"

    MODEL_DIR=$(dirname "$MODEL_PATH")
    echo "${MODEL_DIR}/test.raw" > ./model_tmp/input_list_quant.txt


    {
      echo '{'
      echo '  "graphs": ['
      echo '    {'
      echo '      "graph_names": ['
      echo "        \"${QNN_MODEL_NAME}\""
      echo '      ],'
      echo '      "vtcm_mb": 8'
      echo '    }'
      echo '  ],'
      echo '  "devices": ['
      echo '    {'
      echo "      \"htp_arch\": \"${HTP_VERSION}\""
      echo '    }'
      echo '  ]'
      echo '}'
    } > ./model_tmp/htp_config.json

    {
      echo '{'
      echo '  "backend_extensions": {'
      echo "    \"shared_library_path\": \"${QNN_SDK_ROOT}/lib/${QNN_TARGET_ARCH}/libQnnHtpNetRunExtensions.so\","
      echo "    \"config_file_path\": \"${CURRENT_DIR}/model_tmp/htp_config.json\""
      echo '  }'
      echo '}'
    } > ./model_tmp/htp_backend_extensions.json
    

    python3 "${QNN_SDK_ROOT}/bin/x86_64-linux-clang/qnn-pytorch-converter" \
    --input_network "${MODEL_PATH}" \
    --input_dim input $BATCH_NUM,$CHANNELS,$HEIGHT,$WIDTH \
    -o ./model_tmp/$QNN_MODEL_CPP_NAME \
    --input_list ./model_tmp/input_list_quant.txt

    python3 "${QNN_SDK_ROOT}/bin/x86_64-linux-clang/qnn-model-lib-generator" \
    -c ./model_tmp/${QNN_MODEL_CPP_NAME} \
    -b ./model_tmp/${QNN_MODEL_BIN_NAME} \
    -o ./model_tmp/quant \
    -t $QNN_TARGET_ARCH

    ${QNN_SDK_ROOT}/bin/${QNN_TARGET_ARCH}/qnn-context-binary-generator \
    --backend "${QNN_SDK_ROOT}/lib/${QNN_TARGET_ARCH}/libQnnHtp.so" \
    --model "./model_tmp/quant/${QNN_TARGET_ARCH}/lib${QNN_MODEL_NAME}.so" \
    --binary_file "${CURRENT_DIR}/model_tmp/lib${QNN_MODEL_NAME}.serialized" \
    --config_file "./model_tmp/htp_backend_extensions.json"


else
    echo "Unsupported backend: $BACKEND"
    exit 1
fi

echo "/data/local/tmp/qnnx/test.raw" > ./model_tmp/input_list.txt
