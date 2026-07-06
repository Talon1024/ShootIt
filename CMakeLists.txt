cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(SOURCE_DIR src)
project(ShootIt VERSION 0.1)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

add_executable(Game)

target_sources(Game PRIVATE ${SOURCE_DIR}/main.cpp)
target_link_libraries(Game ${SDL3_LIBRARIES})

file(GLOB_RECURSE ASSETS LIST_DIRECTORIES false RELATIVE ${PROJECT_SOURCE_DIR} assets/*.png)
if(EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --use-port=sdl3")
    foreach(asset IN LISTS ASSETS)
        # The alias needs to be set because the assets are stored outside of
        # the build directory. The alias is the path from the project root to
        # the file.
        target_link_options(Game PRIVATE "SHELL:--embed-file ${PROJECT_SOURCE_DIR}/${asset}@${asset}")
    endforeach()
    set_property(TARGET Game PROPERTY OUTPUT_NAME index)
    set_property(TARGET Game PROPERTY SUFFIX ".html")
    target_link_options(Game PRIVATE -sALLOW_MEMORY_GROWTH=1)
    target_link_options(Game PRIVATE "SHELL:--shell-file ${PROJECT_SOURCE_DIR}/wsrc/shell.html")
    if (EMRUN)
        message(STATUS "emrun enabled")
        target_link_options(Game PRIVATE "SHELL:--emrun")
    endif()
    file(COPY assets/favicon.ico DESTINATION ${PROJECT_BINARY_DIR})
    file(COPY wsrc/testharness.html DESTINATION ${PROJECT_BINARY_DIR})
endif()

# configure_file(TutorialConfig.h.in TutorialConfig.h)
target_include_directories(Game PUBLIC "${PROJECT_BINARY_DIR}")
