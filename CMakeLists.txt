cmake_minimum_required(VERSION 3.16)
project(TowerDefense LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(TowerDefense
    Map.cpp
    Critter.cpp
    CritterGroup.cpp
    CritterGroupGenerator.cpp
    Tower.cpp
    Observable.cpp
)

target_include_directories(TowerDefense PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# The driver runs the tests, then opens the SFML game window. SFML is
# downloaded and built automatically the first time CMake configures the
# project (needs an internet connection once).
include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(SFML)

add_executable(TowerDefenseDriver
    Driver.cpp
    Gui.cpp
    CritterObserver.cpp
)

# SFML 3 requires C++17; the library itself stays C++11.
target_compile_features(TowerDefenseDriver PRIVATE cxx_std_17)
target_link_libraries(TowerDefenseDriver PRIVATE TowerDefense SFML::Graphics)
