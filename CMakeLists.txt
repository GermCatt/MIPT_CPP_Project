cmake_minimum_required(VERSION 3.21)

project(cpp_project LANGUAGES CXX)

option(TRAIN_NETWORK "Build the neural network training program" OFF)
set(TRAINING_SOURCES
    neural_network/training/training_example.hpp
    neural_network/training/training_example.cpp
    neural_network/training/shot_policy.hpp
    neural_network/training/shot_policy.cpp
)

if(TRAIN_NETWORK)
    set(APP_MAIN
        neural_network/training/train.cpp
        neural_network/training/network_gradients.cpp
        neural_network/training/loss.hpp
        neural_network/training/loss.cpp
        ${TRAINING_SOURCES}
    )
else()
    set(APP_MAIN main.cpp
            common/game_types.hpp
            common/board_snapshot.hpp
            common/player_view.hpp
            common/Field.hpp
            Interfaces/IGameField.hpp
            Interfaces/IShotInput.hpp
            Interfaces/IGameOutput.hpp
            Game/Game.cpp
            Game/Game.hpp
            common/board_snapshot.cpp
            Game/BoardStage.cpp
            Game/BoardStage.hpp
            common/player_view.cpp
            Interfaces/IPlacementProvider.hpp
            Placement/RandomPlacementProvider.cpp
            Placement/RandomPlacementProvider.hpp
            Profiles/PlayerProfile.hpp
            common/PlayerKind.hpp
            Profiles/PlayerProfile.cpp
            Profiles/ProfileManager.cpp
            Profiles/ProfileManager.hpp
            Interfaces/IProfileInput.hpp
            Profiles/ProfileRepository.hpp
            Profiles/ProfileRepository.cpp
    )
endif()

set(COMMON_SOURCES
    Game/board_input.hpp
    Game/board_input.cpp
    neural_network/network_input.hpp
    neural_network/network_input.cpp
    Game/board.hpp
    Game/board.cpp
    neural_network/placement_generator.hpp
    neural_network/placement_generator.cpp
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
)

set(HARD_NETWORK neural_network/hard_network.cpp neural_network/hard_network.hpp neural_network/local_wound.hpp)
add_executable(app ${APP_MAIN} Game/computer_opponent.hpp Game/computer_opponent.cpp ${HARD_NETWORK} ${COMMON_SOURCES})
add_executable(local_train EXCLUDE_FROM_ALL
    neural_network/training/local_train.cpp
    neural_network/training/layout_io.hpp
    ${TRAINING_SOURCES}
    neural_network/training/hard_network_trainer.hpp
    neural_network/training/hard_network_trainer.cpp
    ${HARD_NETWORK} ${COMMON_SOURCES}
)

foreach(target IN ITEMS app local_train)
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
