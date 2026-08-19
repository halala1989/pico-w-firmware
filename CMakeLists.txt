cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_w_keyboard_firmware C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

pico_sdk_init()

# BLE GATT server compiled from pico_w_keyboard.gatt (BTstack).
pico_btstack_ble(pico_w_keyboard_firmware
    TARGET_LIB pico_btstack_ble_pico_w_keyboard
    GATT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/pico_w_keyboard.gatt
)

add_executable(pico_w_keyboard_firmware
    src/main.c
    src/packet_parser.c
    src/usb_hid.c
)

target_include_directories(pico_w_keyboard_firmware PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Console over UART only; stdio USB must stay OFF so TinyUSB can act as HID.
pico_enable_stdio_uart(pico_w_keyboard_firmware ON)
pico_enable_stdio_usb(pico_w_keyboard_firmware OFF)
pico_enable_usb_reset_interface(pico_w_keyboard_firmware OFF)

target_link_libraries(pico_w_keyboard_firmware
    pico_cyw43_arch_lwip_threadsafe_background
    pico_btstack_cyw43
    pico_btstack_ble_pico_w_keyboard
    tinyusb_device
    hardware_timer
)

pico_add_extra_outputs(pico_w_keyboard_firmware)
