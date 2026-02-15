cmake_minimum_required(VERSION 3.14)

project(ReactiveLitepp VERSION 1.0.0 LANGUAGES CXX)
find_package(nameof CONFIG REQUIRED)

# Header-only library
add_library(ReactiveLitepp INTERFACE)

target_include_directories(ReactiveLitepp INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(ReactiveLitepp INTERFACE cxx_std_17)

target_link_libraries(ReactiveLitepp INTERFACE nameof::nameof)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    add_subdirectory(examples)
endif()

# Option to build tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    enable_testing()
    add_subdirectory(tests)
endif()

