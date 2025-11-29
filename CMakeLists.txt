cmake_minimum_required(VERSION 3.13)

# If the Pico SDK is present as a submodule under pico-sdk/, use it.
# Otherwise rely on PICO_SDK_PATH being set by the user.
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/pico-sdk/external/pico_sdk_import.cmake")
    set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/pico-sdk" CACHE PATH "Path to the Pico SDK" FORCE)
elseif(NOT DEFINED PICO_SDK_PATH)
    message(FATAL_ERROR
        "Pico SDK not found. Clone it as a submodule at pico-sdk/ or set PICO_SDK_PATH.")
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(serial_test C CXX ASM)

pico_sdk_init()

add_executable(serial_test
    main.c
    ssd1306.c
)

target_link_libraries(serial_test 
    pico_stdlib
    hardware_i2c
)

pico_enable_stdio_usb(serial_test 0)
pico_enable_stdio_uart(serial_test 1)

pico_add_extra_outputs(serial_test)
