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
)

target_include_directories(TowerDefense PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
