cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(SOURCE_DIR src)
project(ShootItSimpleStupid VERSION 0.1)

# 1. Look for a SDL3 package,
# 2. look for the SDL3-shared component, and
# 3. fail if the shared component cannot be found.
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

if(${CMAKE_TOOLCHAIN_FILE} MATCHES cmake/Modules/Platform/Emscripten\.cmake$)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --use-port=sdl3")
endif()

add_executable(Game)
target_sources(Game PRIVATE ${SOURCE_DIR}/main.cpp)
target_link_libraries(Game ${SDL3_LIBRARIES})
# configure_file(TutorialConfig.h.in TutorialConfig.h)
target_include_directories(Game PUBLIC "${PROJECT_BINARY_DIR}")
