set(PROJECT psion_recreate_ctusb)
cmake_minimum_required(VERSION 3.12)
#set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
set(PICO_SDK_PATH "/home/menadue/pico/pico-sdk")
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(${PROJECT} C CXX ASM)

pico_sdk_init()
add_executable(${PROJECT}
    main.c
    cdc.c
    core1.c
    usb_descriptors.c
    font.c
    svc_kb.c
    svc_dp.c
    svc_ed.c
    svc_fl.c
    svc_pk.c
    svc_er.c
    svc_db.c
    svc_pak_pico_flash.c
    svc_pak_serial_eeprom.c
    display.c
    cursor.c
    rtc.c
    i2c.c
    menu.c
    serial.c
    sysvar.c
    eeprom.c
    panel.c
    opl.c
    msc_disk.c
    ssd1309.c
    ssd1351.c
    oled_er_oledm015_2.c
    display_driver.c
  )
target_include_directories(${PROJECT} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(${PROJECT}
    pico_stdlib
    pico_multicore
    pico_unique_id
    tinyusb_board
    tinyusb_device
)

pico_add_extra_outputs(${PROJECT})
pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)
