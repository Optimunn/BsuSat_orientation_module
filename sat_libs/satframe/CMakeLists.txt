add_library(satframe INTERFACE)

add_subdirectory(../mcp2515 ${CMAKE_BINARY_DIR}/mcp2515)

target_include_directories(satframe
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

target_link_libraries(satframe
    INTERFACE
    pico_stdlib
    mcp2515
)

target_sources(satframe
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/satframe.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inc/satframe.hpp
)