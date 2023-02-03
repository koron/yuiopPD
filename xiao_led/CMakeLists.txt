add_library(xiao_led INTERFACE)

target_include_directories(xiao_led INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(xiao_led INTERFACE led.c)

target_link_libraries(xiao_led INTERFACE
    hardware_gpio
    pico_binary_info
    )

# vim:set ts=4 sts=4 sw=4 tw=0 et:
