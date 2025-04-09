add_library(ecoil INTERFACE)

target_include_directories(ecoil
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

target_link_libraries(ecoil
    INTERFACE
    pico_stdlib
    hardware_pwm
    hardware_clocks
)

target_sources(ecoil
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/ecoil.c
    ${CMAKE_CURRENT_LIST_DIR}/inc/ecoil.h
)