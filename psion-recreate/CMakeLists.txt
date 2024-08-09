cmake_minimum_required(VERSION 3.13)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/tree/downloaded_tools/pico/pico-sdk")

include(pico_sdk_import.cmake)

project(psion_recreate C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(psion_recreate
psion_recreate.c
display.c
font.c
emulator.c
wireless.c
rtc.c
i2c.c
match.c
eeprom.c
menu.c
serial.c
)

pico_enable_stdio_usb(psion_recreate 1)
#pico_enable_stdio_uart(psion_recreate 1)

target_link_libraries(psion_recreate
  pico_stdlib
  pico_multicore
  hardware_flash)

pico_add_extra_outputs(psion_recreate)

