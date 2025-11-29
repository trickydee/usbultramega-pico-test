#!/bin/sh
set -eu

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build-pico"
SDK_DIR="${PROJECT_DIR}/pico-sdk"
SDK_BRANCH="${PICO_SDK_BRANCH:-2.2.0}"

echo ">>> Ensuring Pico SDK is available..."

ensure_sdk_present() {
    if [ -d "${SDK_DIR}/src/rp2_common" ] && [ -f "${SDK_DIR}/pico_sdk_init.cmake" ]; then
        echo "    ✅ Pico SDK found at ${SDK_DIR}"
        return 0
    fi
    return 1
}

if ! ensure_sdk_present; then
    echo "    ⚠️ Pico SDK missing or incomplete. Fetching..."
    rm -rf "${SDK_DIR}"

    if git submodule status >/dev/null 2>&1 && \
       git config --file .gitmodules --get-regexp "^submodule\.." >/dev/null 2>&1; then
        echo "    ↻ Initialising submodules..."
        if git submodule update --init --recursive pico-sdk; then
            ensure_sdk_present && FETCH_OK=1 || FETCH_OK=0
        else
            FETCH_OK=0
        fi
    else
        FETCH_OK=0
    fi

    if [ "${FETCH_OK:-0}" -ne 1 ]; then
        echo "    ⇣ Falling back to git clone (branch ${SDK_BRANCH})..."
        git clone --branch "${SDK_BRANCH}" --recursive \
            https://github.com/raspberrypi/pico-sdk.git "${SDK_DIR}"
    fi

    if ! ensure_sdk_present; then
        echo "ERROR: Failed to obtain Pico SDK. Check network access and try again."
        exit 1
    fi
fi

echo ">>> Checking for ARM GCC toolchain..."
if ! TOOLCHAIN_GCC="$(command -v arm-none-eabi-gcc 2>/dev/null)"; then
    echo "ERROR: arm-none-eabi-gcc not found. Install the ARM GCC toolchain (e.g. 'brew install arm-none-eabi-gcc') and re-run."
    exit 1
fi
TOOLCHAIN_BIN_DIR="$(cd "$(dirname "${TOOLCHAIN_GCC}")" && pwd)"
TOOLCHAIN_ROOT="$(cd "${TOOLCHAIN_BIN_DIR}/.." && pwd)"
export PATH="${TOOLCHAIN_BIN_DIR}:${PATH}"
TOOLCHAIN_ARGS="-DPICO_TOOLCHAIN_PATH=${PICO_TOOLCHAIN_PATH:-${TOOLCHAIN_ROOT}}"

echo "    ✅ Using toolchain at ${TOOLCHAIN_ROOT}"

# If a previous cache exists but points to the wrong compiler, clear it.
if [ -f "${BUILD_DIR}/CMakeCache.txt" ] && ! grep -q "arm-none-eabi-gcc" "${BUILD_DIR}/CMakeCache.txt"; then
    echo ">>> Detected host compiler in existing cache; cleaning ${BUILD_DIR}"
    rm -rf "${BUILD_DIR}"
fi

mkdir -p "${BUILD_DIR}"

echo ">>> Configuring (logs in ${BUILD_DIR}/cmake.log)..."
if ! cmake -S "${PROJECT_DIR}" -B "${BUILD_DIR}" -DPICO_BOARD="${PICO_BOARD:-pico}" \
    ${TOOLCHAIN_ARGS} \
    >"${BUILD_DIR}/cmake.log" 2>&1; then
    echo "ERROR: CMake configuration failed for ${PICO_BOARD:-pico}."
    tail -20 "${BUILD_DIR}/cmake.log" || true
    exit 1
fi

echo ">>> Building (logs in ${BUILD_DIR}/build.log)..."
if ! cmake --build "${BUILD_DIR}" -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)" \
    >"${BUILD_DIR}/build.log" 2>&1; then
    echo "ERROR: Build failed."
    tail -20 "${BUILD_DIR}/build.log" || true
    exit 1
fi

echo ">>> Build complete. Artifacts in ${BUILD_DIR}"