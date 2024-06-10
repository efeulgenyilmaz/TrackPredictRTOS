pico_sdk_init()

add_executable(3data_servo
        main.c
        )

# pull in common dependencies and additional pwm hardware support
target_link_libraries(3data_servo
    pico_stdlib
    hardware_pwm
)
target_link_libraries(3data_servo
    pico_stdlib
    hardware_uart
)
target_link_libraries(3data_servo
    pico_stdlib
    pico_multicore
)

# create map/bin/hex file etc.
pico_add_extra_outputs(3data_servo)

# add url via pico_set_program_url
example_auto_set_url(3data_servo)
