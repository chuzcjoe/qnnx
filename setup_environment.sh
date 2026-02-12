#!/bin/bash

DEFAULT_QNN_TARGET_ARCH="x86_64-linux-clang"

if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
  echo "Usage: source setup_environment.sh [qnn target arch]"
  echo "Valid qnn target arch values: x86_64-linux-clang | aarch64-android"
  echo "If no target arch is provided, default is: ${DEFAULT_QNN_TARGET_ARCH}"
  return 0 2>/dev/null || exit 0
fi

QNN_TARGET_ARCH="${1:-$DEFAULT_QNN_TARGET_ARCH}"

case "${QNN_TARGET_ARCH}" in
  x86_64-linux-clang|aarch64-android)
    ;;
  *)
    echo "Error: invalid qnn target arch '${QNN_TARGET_ARCH}'."
    echo "Expected one of: x86_64-linux-clang | aarch64-android"
    return 1 2>/dev/null || exit 1
    ;;
esac

export QNN_SDK_ROOT="${HOME}/Documents/repos/qnn_sdk/qairt/2.43.0.260128"
export QNN_TARGET_ARCH

echo "QNN_SDK_ROOT set to: ${QNN_SDK_ROOT}"
echo "QNN_TARGET_ARCH set to: ${QNN_TARGET_ARCH}"

source "${QNN_SDK_ROOT}/bin/envsetup.sh"

python3 ./check-python-dependency