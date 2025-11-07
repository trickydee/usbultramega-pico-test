#!/bin/sh
set -eu

################################################################################
# Initialize git submodules
################################################################################

echo ">>> Checking Pico SDK..."

# Check if pico-sdk exists and has actual content
PICO_SDK_OK=false
if [ -d "./pico-sdk/src" ] && [ -f "./pico-sdk/pico_sdk_init.cmake" ]; then
    # Check if it's actually populated (has more than just error files)
    if [ -d "./pico-sdk/src/rp2_common" ]; then
        PICO_SDK_OK=true
        echo "    ✅ Pico SDK is properly initialized"
    fi
fi

# If pico-sdk is missing or incomplete, reinitialize it
if [ "$PICO_SDK_OK" = false ]; then
    echo "    ⚠️  Pico SDK missing or incomplete - reinitializing..."

    # Clean up any partial/broken pico-sdk directory
    if [ -d "./pico-sdk" ]; then
        echo "    Removing incomplete pico-sdk directory..."
        rm -rf ./pico-sdk
    fi

    # Initialize submodules
    echo "    Cloning Pico SDK and dependencies (this may take a minute)..."
    if ! git submodule update --init --recursive; then
        echo "ERROR: Failed to initialize git submodules!"
        echo "Please check your internet connection and try again."
        exit 1
    fi

    echo "    ✅ Pico SDK initialized successfully"
fi


PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

if [ ! -d "${BUILD_DIR}" ]; then
    cmake -S "${PROJECT_DIR}" -B "${BUILD_DIR}"
fi

cmake --build "${BUILD_DIR}" "$@"

