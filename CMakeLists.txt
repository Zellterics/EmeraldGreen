cmake_minimum_required(VERSION 3.10)
project(test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TEST_ROOT_DIR "${CMAKE_SOURCE_DIR}")

set(THING_SHADER_OVERRIDES
    "basicVert=${TEST_ROOT_DIR}/shaders/basicVert.spv;basicFrag=${TEST_ROOT_DIR}/shaders/basicFrag.spv"
    CACHE STRING "" FORCE
)

add_subdirectory(ThING-Engine)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    graph/*.cpp
)

add_executable(test 
    main.cpp
    ${SOURCES}
)
target_link_libraries(test PRIVATE ThING)
