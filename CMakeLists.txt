cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_home_security C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico_home_security
            libs/mfrc522.c
            libs/easytcp.c
            src/main.c
            )

pico_enable_stdio_usb(pico_home_security 0)
pico_enable_stdio_uart(pico_home_security 1)

pico_add_extra_outputs(pico_home_security)

target_include_directories(pico_home_security PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR} 
    src/
    libs/
    )

target_link_libraries(pico_home_security 
            pico_cyw43_arch_lwip_threadsafe_background 
            pico_stdlib 
            pico_multicore 
            hardware_spi
            )