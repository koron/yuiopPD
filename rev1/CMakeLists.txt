add_executable(yuiopPD_rev1
    main.c
    tud_callbacks.c
	usb_descriptors.c
    )

pico_set_program_name(yuiopPD_rev1 "yuiopPD rev.1")
pico_set_program_url(yuiopPD_rev1 "https://github.com/koron/yuiopPD")
pico_set_program_description(yuiopPD_rev1 "DIY pointing device")

pico_enable_stdio_uart(yuiopPD_rev1 1)
pico_enable_stdio_usb(yuiopPD_rev1 0)

target_include_directories(yuiopPD_rev1 PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_definitions(yuiopPD_rev1 PRIVATE
    PICO_DEFAULT_UART_TX_PIN=0
    PICO_DEFAULT_UART_RX_PIN=-1
    PICO_DEFAULT_SPI_CSN_PIN=1
)

target_link_libraries(yuiopPD_rev1
    hardware_spi
    pico_binary_info
    pico_stdlib
    pmw3360
    tinyusb_device
    )

pico_add_extra_outputs(yuiopPD_rev1)

# vim:set ts=4 sts=4 sw=4 tw=0 et:
