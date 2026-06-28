add_executable(testapp testapp.c)

target_sources(testapp PRIVATE testapp.c 
               nvconfig.c
               usb_main.c
               usb_descriptors.c
               microrl/src/microrl.c
               microrl/examples/example.c
               microrl/examples/example_misc.c
               )

target_link_libraries(testapp PRIVATE pico_stdlib)
# pico_add_extra_outputs(testapp)

# add url via pico_set_program_url
# example_auto_set_url(testapp)

set(CMAKE_BUILD_TYPE Debug)

# USB section
set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")
# Provide an LWIP path for net_lwip_webserver in case the module does not exist in tinyusb
set(TINYUSB_LWIP_PATH ${PICO_LWIP_PATH})
# Some examples use this, and we need to set this here due to a bug in the TinyUSB CMake config
set(TOP ${PICO_TINYUSB_PATH})

target_include_directories(testapp PUBLIC ${CMAKE_CURRENT_LIST_DIR}
                                          ${CMAKE_CURRENT_LIST_DIR}/microrl/src
                                          ${CMAKE_CURRENT_LIST_DIR}/microrl/examples
                                          ${CMAKE_CURRENT_LIST_DIR}/microrl/examples/unix_misc
                                          )

# target_compile_definitions(testapp PRIVATE CFG_TUSB_MCU=OPT_MCU_RP2040 )

target_link_libraries(testapp PRIVATE pico_stdlib pico_unique_id tinyusb_device tinyusb_board)

# コードFLASH領域の上限を256KBに設定する
pico_set_linker_script(testapp ${CMAKE_CURRENT_LIST_DIR}/memmap_custom.ld)

pico_add_extra_outputs(testapp)

# add url via pico_set_program_url
example_auto_set_url(testapp)
