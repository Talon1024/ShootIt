cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(SOURCE_DIR src)
project(ShootIt VERSION 0.1)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

add_executable(Game)

target_sources(Game PRIVATE ${SOURCE_DIR}/main.cpp)
target_link_libraries(Game ${SDL3_LIBRARIES})

if(EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --use-port=sdl3")
    set_property(TARGET Game PROPERTY SUFFIX ".html")
    target_link_options(Game PRIVATE -sALLOW_MEMORY_GROWTH=1)
endif()

# configure_file(TutorialConfig.h.in TutorialConfig.h)
target_include_directories(Game PUBLIC "${PROJECT_BINARY_DIR}")
