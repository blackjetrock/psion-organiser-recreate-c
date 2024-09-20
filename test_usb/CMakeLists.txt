project(dev_hid_composite)

set(FAMILY rp2040)
set(BOARD pico_sdk)
set(PICO_BOARD pico)
set(PICO_SDK_PATH "/home/menadue/pico/pico-sdk")

include(pico_sdk_import.cmake)
project(psion_recreate C CXX ASM)

set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")

# Provide an LWIP path for net_lwip_webserver in case the module does not exist in tinyusb
set(TINYUSB_LWIP_PATH ${PICO_LWIP_PATH})

# Some examples use this, and we need to set this here due to a bug in the TinyUSB CMake config
set(TOP ${PICO_TINYUSB_PATH})

#add_subdirectory(${PICO_TINYUSB_PATH}/examples/device tinyusb_device_examples)
#add_subdirectory_exclude_platforms(dev_hid_composite)
#add_subdirectory_exclude_platforms(dev_lowlevel)

cmake_minimum_required(VERSION 3.13)

add_executable(dev_hid_composite)

target_sources(dev_hid_composite PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(dev_hid_composite PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(dev_hid_composite PUBLIC pico_stdlib pico_unique_id tinyusb_device tinyusb_board)

# Uncomment this line to enable fix for Errata RP2040-E5 (the fix requires use of GPIO 15)
#target_compile_definitions(dev_hid_composite PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

pico_add_extra_outputs(dev_hid_composite)

# add url via pico_set_program_url
example_auto_set_url(dev_hid_composite)
