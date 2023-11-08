# Copyright (C) G2Labs Grzegorz Grzęda - All Rights Reserved
# Unauthorized copying of this file, via any medium is strictly prohibited
# Proprietary and confidential
# Written by Grzegorz Grzęda <grzegorz.grzeda@g2labs.pl>, 03.2023
cmake_minimum_required(VERSION 3.22)
enable_testing()

project(ssd1306)
add_library(${PROJECT_NAME})

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(${CMAKE_CURRENT_LIST_DIR}/lib/atomic-tests/cmake/atomic-tests.cmake)
    add_subdirectory(lib/generic-controller)
    add_subdirectory(lib/generic-display)
endif()

add_subdirectory(source)
add_subdirectory(tests)

target_link_libraries(${PROJECT_NAME}
    PUBLIC generic-display
    PUBLIC generic-controller
)
