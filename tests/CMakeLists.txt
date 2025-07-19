add_executable(conv_test
    conv_test.c
    noise.c
    codes.c
)

target_link_libraries(conv_test PRIVATE turbofec pthread m)

add_executable(turbo_test
    turbo_test.c
    noise.c
)

target_link_libraries(turbo_test PRIVATE turbofec pthread m)

add_executable(ber_sweep
    ber_sweep.c
    noise.c
)

target_link_libraries(ber_sweep PRIVATE turbofec pthread m)

add_test(NAME conv_test COMMAND conv_test)
add_test(NAME turbo_test COMMAND turbo_test)
add_test(NAME ber_sweep COMMAND ber_sweep)
