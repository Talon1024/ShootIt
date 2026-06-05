cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(SOURCE_DIR src)
project(ShootItSimpleStupid VERSION 0.1)

find_package(SDL3)

add_executable(Game)
target_sources(Game PRIVATE ${SOURCE_DIR}/main.cpp)
target_link_libraries(Game ${SDL3_LIBRARIES})
# configure_file(TutorialConfig.h.in TutorialConfig.h)
target_include_directories(Game PUBLIC "${PROJECT_BINARY_DIR}")
