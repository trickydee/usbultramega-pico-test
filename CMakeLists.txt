cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
set(PICO_SDK_PATH ${CMAKE_SOURCE_DIR}/pico-sdk)
include(pico_sdk_import.cmake)

project(serial_test C CXX ASM)

pico_sdk_init()

add_executable(serial_test
    main.c
)

target_link_libraries(serial_test pico_stdlib)

pico_enable_stdio_usb(serial_test 0)
pico_enable_stdio_uart(serial_test 1)

pico_add_extra_outputs(serial_test)

