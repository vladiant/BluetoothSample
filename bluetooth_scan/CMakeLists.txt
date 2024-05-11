cmake_minimum_required(VERSION 3.10)

project(BluetoothScan LANGUAGES C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(PC_BLUETOOTH bluez)

find_path(BLUETOOTH_INCLUDE_DIR NAMES bluetooth/bluetooth.h
                                PATHS ${PC_BLUETOOTH_INCLUDEDIR})
find_library(BLUETOOTH_LIBRARY NAMES bluetooth libbluetooth
                               PATHS ${PC_BLUETOOTH_LIBDIR})

add_executable(
    ${PROJECT_NAME}
        main.c
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        ${PC_BLUETOOTH_INCLUDEDIR}
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        ${BLUETOOTH_LIBRARY}
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
        LINKER_LANGUAGE C
)

target_compile_options(
    ${PROJECT_NAME}
    PUBLIC
         -Wall -Wextra -Wpedantic
)
