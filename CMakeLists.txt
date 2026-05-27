cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME project_1)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
        User/App/app_data.c
        User/App/app_data.h
        User/Bsp/bsp_dht11.c
        User/Bsp/bsp_dht11.h
        User/Bsp/bsp_oled.c
        User/Bsp/bsp_oled.h
        User/Bsp/bsp_esp32c6.c
        User/Bsp/bsp_esp32c6.h
        User/Bsp/bsp_light.c
        User/Bsp/bsp_light.h
        User/Common/led.c
        User/Common/led.h
        User/Common/button.c
        User/Common/button.h
        User/Service/service_dht11.c
        User/Service/service_dht11.h
        User/Service/service_uart.c
        User/Service/service_uart.h
        User/Service/service_oled.c
        User/Service/service_oled.h
        User/Service/service_esp32c6.c
        User/Service/service_esp32c6.h
        User/Service/service_light.c
        User/Service/service_light.h
        User/Service/service_command.c
        User/Service/service_command.h
        User/Service/service_config.c
        User/Service/service_config.h
        User/Tasks/task_dht11.c
        User/Tasks/task_dht11.h
        User/Tasks/task_uart.c
        User/Tasks/task_uart.h
        User/Tasks/task_oled.c
        User/Tasks/task_oled.h
        User/Tasks/task_esp32c6.c
        User/Tasks/task_esp32c6.h
        User/Tasks/task_light.c
        User/Tasks/task_light.h
        User/Common/delay.c
        User/Common/delay.h
        User/COmmon/oledfont.h)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/User/App
    ${CMAKE_SOURCE_DIR}/User/Bsp
    ${CMAKE_SOURCE_DIR}/User/Service
    ${CMAKE_SOURCE_DIR}/User/Tasks
    ${CMAKE_SOURCE_DIR}/User/Common
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# Add linker options
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -u _printf_float
)
