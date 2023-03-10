add_executable(yuiopPD_rev1
    main.c
    tud_callbacks.c
    usb_descriptors.c
    )

pico_set_program_name(yuiopPD_rev1 "yuiopPD rev.1")
pico_set_program_url(yuiopPD_rev1 "https://github.com/koron/yuiopPD")
pico_set_program_description(yuiopPD_rev1 "DIY pointing device")

pico_enable_stdio_uart(yuiopPD_rev1 0)
pico_enable_stdio_usb(yuiopPD_rev1 1)

target_include_directories(yuiopPD_rev1 PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_compile_definitions(yuiopPD_rev1 PRIVATE
    PICO_DEFAULT_UART_TX_PIN=-1
    PICO_DEFAULT_UART_RX_PIN=-1
    PICO_DEFAULT_SPI_CSN_PIN=1
)

target_link_libraries(yuiopPD_rev1
    direct_button
    pmw3360
    xiao_led
    usb_mouse
    heartbeat

    hardware_spi
    pico_binary_info
    pico_bootsel_via_double_reset
    pico_stdlib
    tinyusb_device
    )

pico_add_extra_outputs(yuiopPD_rev1)

# vim:set ts=4 sts=4 sw=4 tw=0 et:
