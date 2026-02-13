#!/usr/bin/env bash
set -e

echo "usage: $0 <path-to-pytorch-model> <batch num> <channels> <height> <width> <beackend>"

MODEL_PATH=$1
BATCH_NUM=$2
CHANNELS=$3
HEIGHT=$4
WIDTH=$5
BACKEND=$6

rm -rf model_tmp
mkdir -p model_tmp

echo "QNN_SDK_ROOT: ${QNN_SDK_ROOT}"

if [ "$BACKEND" = "cpu" ] || [ "$BACKEND" = "gpu" ]; then
    echo "Converting Pytorch model to FP32 QNN model"

    TORCH_MODEL_NAME=$(basename "$MODEL_PATH" .pt)
    QNN_MODEl_NAME="${TORCH_MODEL_NAME}_fp32"

    QNN_MODEL_CPP_NAME="${QNN_MODEl_NAME}.cpp"
    QNN_MODEL_BIN_NAME="${QNN_MODEl_NAME}.bin"

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
else
    echo "Unsupported backend: $BACKEND"
    exit 1
fi

