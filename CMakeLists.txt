# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.19)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

get_filename_component(BASE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PROJECT_NAME ${BASE_FOLDER})

add_subdirectory(sat_libs/satframe ${CMAKE_BINARY_DIR}/satframe)
add_subdirectory(sat_libs/mp92plus ${CMAKE_BINARY_DIR}/mp92plus)
add_subdirectory(sat_libs/ecoil ${CMAKE_BINARY_DIR}/ecoil)
add_subdirectory(sat_libs/ina3221 ${CMAKE_BINARY_DIR}/ina3221)

# Set name of project (as PROJECT_NAME) and C/C   standards
project(${PROJECT_NAME} C CXX ASM)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME} 
    main.cpp
	func_plus.cpp
)

# Create map/bin/hex/uf2 files
pico_add_uf2_output(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    satframe
	ecoil
	mp92plus
	ina3221
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0) 

add_custom_command(TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E echo ""
	COMMAND ${PICO_GCC_TRIPLE}-size -B --format=gnu $<TARGET_FILE_NAME:${PROJECT_NAME}>
	COMMAND ${CMAKE_COMMAND} -E echo ""
	#COMMAND ${PICO_GCC_TRIPLE}-objdump $<TARGET_FILE_NAME:${PROJECT_NAME}> -h
)