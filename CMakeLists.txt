cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(yuiopPD C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD seeed_xiao_rp2040)

pico_sdk_init()

add_subdirectory(direct_button)
add_subdirectory(pmw3360)
add_subdirectory(rev1)

# vim:set ts=8 sts=4 sw=4 tw=0 et:
