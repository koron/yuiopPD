add_library(usb_mouse INTERFACE)

target_include_directories(usb_mouse INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(usb_mouse INTERFACE mouse.c)

target_link_libraries(direct_button INTERFACE
    tinyusb_device
    )

# vim:set ts=4 sts=4 sw=4 tw=0 et:
