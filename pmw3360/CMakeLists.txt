add_library(pmw3360 INTERFACE)

target_include_directories(pmw3360 INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(pmw3360 INTERFACE
    pmw3360.c
    srom_0x04.c
    srom_0x81.c
    )

target_link_libraries(pmw3360 INTERFACE
    hardware_spi
    )

# vim:set ts=4 sts=4 sw=4 tw=0 et:
