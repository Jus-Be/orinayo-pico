# BLE Client Example CMakeLists.txt
cmake_minimum_required(VERSION 3.12)

# Include the parent directory's CMake configuration
include(../CMakeLists.txt)

# Create a separate executable for the BLE client example
add_executable(ble_client_example ble_client_example.c)

# Include directories from parent
target_include_directories(ble_client_example PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/.. 
    ${CMAKE_CURRENT_LIST_DIR}/../bluepad32/include 
    ${PICO_SDK_PATH}/lib/btstack/src
)

# Link the same libraries as the main project
target_link_libraries(ble_client_example 
    pico_stdlib 
    pico_multicore 
    pico_cyw43_arch_none 
    pico_cyw43_arch_threadsafe_background 
    pico_btstack_classic 
    pico_btstack_ble 
    pico_btstack_cyw43 
    bluepad32
)

# Enable stdio over UART for debugging (not USB since main app uses USB for MIDI)
pico_enable_stdio_usb(ble_client_example 0)
pico_enable_stdio_uart(ble_client_example 1)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(ble_client_example)

# Set program information
pico_set_program_name(ble_client_example "BLE Client Example")
pico_set_program_description(ble_client_example "Simple BLE client scanner example using bluepad32")