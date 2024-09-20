cmake_minimum_required(VERSION 3.13)
set(PICO_BOARD pico)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/menadue/pico/pico-sdk")

set(PICO_STACK_SIZE, 65536)
set(PICO_USE_STACK_GUARDS 1)

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
paged_eeprom.c
menu.c
serial.c
svc_ed.c
svc_kb.c
svc_dp.c
svc_er.c
svc_fl.c
svc_pk.c
svc_db.c
svc_pak_pico_flash.c
svc_pak_serial_eeprom.c
sysvar.c
sleep.c
rosc.c
usb_descriptors.c
msc_disk.c
#usb_utility.c
)

pico_enable_stdio_usb(psion_recreate 1)
#pico_enable_stdio_uart(psion_recreate 1)

target_link_libraries(psion_recreate
  pico_stdlib
  pico_multicore
  hardware_rtc
  hardware_clocks
  hardware_flash
  pico_unique_id
  tinyusb_device
  tinyusb_board
  )

target_include_directories(psion_recreate PRIVATE
                        .
                        $ENV{PICO_SDK_PATH}/lib/tinyusb/hw
                        $ENV{PICO_SDK_PATH}/lib/tinyusb/src
                        )

pico_add_extra_outputs(psion_recreate)

