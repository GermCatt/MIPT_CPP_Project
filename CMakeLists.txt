cmake_minimum_required(VERSION 3.21)

project(cpp_project LANGUAGES CXX)

add_executable(app
    main.cpp
    neural_network/instruments/matrix.cpp
    neural_network/instruments/matrix.hpp
    common/errors.hpp
)

target_include_directories(app PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(app PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

if(MSVC)
    target_compile_options(app PRIVATE /W4 /utf-8)
else()
    target_compile_options(app PRIVATE -Wall -Wextra -Wpedantic)
endif()
