cmake_minimum_required(VERSION 3.14)

project(ReactiveLitepp VERSION 1.0.0 LANGUAGES CXX)

add_library(ReactiveLitepp
    src/Subscription.cpp
    src/ScopedSubscription.cpp
)

target_include_directories(ReactiveLitepp PUBLIC include)

target_compile_features(ReactiveLitepp PUBLIC cxx_std_17)

endif()
