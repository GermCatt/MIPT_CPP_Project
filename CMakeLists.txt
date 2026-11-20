cmake_minimum_required(VERSION 3.21)

project(cpp_project LANGUAGES CXX)

option(TRAIN_NETWORK "Build the neural network training program" ON)
if(TRAIN_NETWORK)
    set(APP_MAIN neural_network/train.cpp)
else()
    set(APP_MAIN main.cpp)
endif()

set(COMMON_SOURCES
    Game/board.hpp
    Game/board.cpp
    neural_network/placement_generator.hpp
    neural_network/placement_generator.cpp
    neural_network/training_example.hpp
    neural_network/training_example.cpp
    neural_network/instruments/matrix.cpp
    neural_network/dense_layer.cpp
    neural_network/instruments/activations.cpp
    neural_network/instruments/matrix.hpp
    neural_network/dense_layer.hpp
    neural_network/instruments/activations.hpp
    common/errors.hpp
    common/CellType.hpp
    common/Config.hpp
    common/validate.hpp
    common/validate.cpp
    common/weights.hpp
    common/weights.cpp
    neural_network/neural_network.cpp
    neural_network/neural_network.hpp
    neural_network/instruments/loss.cpp
    neural_network/instruments/loss.hpp
)

add_executable(app ${APP_MAIN} ${COMMON_SOURCES})
add_executable(evaluate neural_network/evaluate.cpp ${COMMON_SOURCES})

foreach(target IN ITEMS app evaluate)
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
