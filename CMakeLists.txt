cmake_minimum_required(VERSION 3.10)

project(BluetoothExample LANGUAGES C)

find_package(ALSA REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(PC_BLUETOOTH bluez)

find_path(BLUETOOTH_INCLUDE_DIR NAMES bluetooth/bluetooth.h
                                PATHS ${PC_BLUETOOTH_INCLUDEDIR})
find_library(BLUETOOTH_LIBRARY NAMES bluetooth libbluetooth
                               PATHS ${PC_BLUETOOTH_LIBDIR})

# BtCommon
add_library(
    BtCommon
    btinclude.h
)

set_target_properties(
    BtCommon
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
        LINKER_LANGUAGE C
)

target_compile_options(
    BtCommon
    PUBLIC
         -Wall -Wextra -Wpedantic
)

target_include_directories(
    BtCommon
    PUBLIC
        ${PC_BLUETOOTH_INCLUDEDIR}
)

target_link_libraries(
    BtCommon
    PUBLIC
        ${BLUETOOTH_LIBRARY}
)

# bthc
add_executable(
    bthc
    bths.c
)

target_include_directories(
    bthc
    PUBLIC
        ${ALSA_INCLUDE_DIRS}
)

target_link_libraries(
    bthc
    PUBLIC
        BtCommon
        ${ALSA_LIBRARIES}
)

set_target_properties(
    bthc
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS YES
        LINKER_LANGUAGE C
)

# rfcomm
add_executable(
    rfcomm
        rfcomm.c
)

target_link_libraries(
    rfcomm
    PUBLIC
        BtCommon
)

set_target_properties(
    rfcomm
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
        LINKER_LANGUAGE C
)

# sco
add_executable(
    sco
        sco.c
)

target_link_libraries(
    sco
    PUBLIC
        BtCommon
)

set_target_properties(
    sco
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
        LINKER_LANGUAGE C
)

# sdp
add_executable(
    sdp
        sdp.c
)

target_link_libraries(
    sdp
    PUBLIC
        BtCommon
)

set_target_properties(
    sdp
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
        LINKER_LANGUAGE C
)

# set_class
add_executable(
    set_class
        set_class.c
)

target_link_libraries(
    set_class
    PUBLIC
        BtCommon
)

set_target_properties(
    set_class
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
        LINKER_LANGUAGE C
)
