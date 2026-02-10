cmake_minimum_required(VERSION 3.14)

project(ReactiveLitepp VERSION 1.0.0 LANGUAGES CXX)

add_library(ReactiveLitepp
    src/Subscription.cpp
    src/ScopedSubscription.cpp
)

target_include_directories(ReactiveLitepp PUBLIC include)

target_compile_features(ReactiveLitepp PUBLIC cxx_std_17)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    add_subdirectory(examples)
endif()

# Option to build tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    enable_testing()
    add_subdirectory(tests)
endif()
