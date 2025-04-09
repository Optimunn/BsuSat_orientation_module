add_library(satframe INTERFACE)

target_include_directories(satframe
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${CMAKE_CURRENT_LIST_DIR}/../mcp2515/include/mcp2515
)

target_link_libraries(satframe
    INTERFACE
    pico_stdlib
    pico_time
    hardware_spi
)

target_sources(satframe
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/../mcp2515/include/mcp2515/mcp2515.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../mcp2515/include/mcp2515/mcp2515.h
    ${CMAKE_CURRENT_LIST_DIR}/../mcp2515/include/mcp2515/can.h
    ${CMAKE_CURRENT_LIST_DIR}/satframe.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inc/satframe.hpp
)