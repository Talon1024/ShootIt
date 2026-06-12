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
    file(GLOB_RECURSE ASSETS LIST_DIRECTORIES false RELATIVE ${PROJECT_SOURCE_DIR} assets/*.png)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --use-port=sdl3")
    foreach(asset IN LISTS ASSETS)
        # The Makefile is usually in the "webuild" directory.
        target_link_options(Game PRIVATE "SHELL:--embed-file ${PROJECT_SOURCE_DIR}/${asset}@${asset}")
    endforeach()
    set_property(TARGET Game PROPERTY SUFFIX ".html")
    target_link_options(Game PRIVATE -sALLOW_MEMORY_GROWTH=1)
endif()

# configure_file(TutorialConfig.h.in TutorialConfig.h)
target_include_directories(Game PUBLIC "${PROJECT_BINARY_DIR}")
