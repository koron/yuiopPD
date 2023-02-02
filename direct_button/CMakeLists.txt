add_library(direct_button INTERFACE)

target_include_directories(direct_button INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(direct_button INTERFACE
    direct_button.c
    )

target_link_libraries(direct_button INTERFACE
    hardware_gpio
    )

# vim:set ts=4 sts=4 sw=4 tw=0 et:
