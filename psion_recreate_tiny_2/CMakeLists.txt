set(PROJECT psion_recreate_ctusb)
cmake_minimum_required(VERSION 3.12)
#set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})

# set for pico board
set(PICO_BOARD pico CACHE STRING "Board type")

set(PICO_SDK_PATH "/home/menadue/pico/pico-sdk")
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(${PROJECT} C CXX ASM)

pico_sdk_init()

add_subdirectory(../no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)
#add_subdirectory(../no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build)

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
    svc_pak_linux.c
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
    sdcard.c
    newopl_lib.c
    newopl_types.c 
    qcode.c 
    time_nopl.c 
    calc.c 
    #clock_pc.c 
    num_a.c 
    machine.c 
    logical_files.c 
    errors.c 
    #svc_pak_linux.c 
    #svc_pk.c 
    #svc_fl.c 
    #svc_kb.c 
    svc_mn.c 
    sysvar.c
    clock_pico.c
    nopl_exec.c
    parser.c
    #nopl_trans.c
    i2ckbd.c
    lcdspi.c
  )
target_include_directories(${PROJECT} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(${PROJECT}
    pico_stdlib
    pico_multicore
    pico_unique_id
    hardware_i2c
    tinyusb_board
    tinyusb_device
    FatFs_SPI
    #no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
  )

#target_link_libraries(_my_app_ FatFs_SPI)

pico_add_extra_outputs(${PROJECT})
pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)
