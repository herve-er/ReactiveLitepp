cmake_minimum_required(VERSION 3.14)

project(ReactiveLitepp VERSION 1.0.0 LANGUAGES CXX)
find_package(nameof CONFIG REQUIRED)

# Header-only library
add_library(ReactiveLitepp INTERFACE)
add_library(ReactiveLitepp::ReactiveLitepp ALIAS ReactiveLitepp)

target_include_directories(ReactiveLitepp INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(ReactiveLitepp INTERFACE cxx_std_20)

target_link_libraries(ReactiveLitepp INTERFACE nameof::nameof)

# Install targets
include(GNUInstallDirs)

install(TARGETS ReactiveLitepp
    EXPORT ReactiveLitepp-targets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ReactiveLitepp-targets
    FILE ReactiveLitepp-targets.cmake
    NAMESPACE ReactiveLitepp::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ReactiveLitepp
)

# Create and install package config files
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ReactiveLitepp-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ReactiveLitepp-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ReactiveLitepp-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ReactiveLitepp
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ReactiveLitepp-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ReactiveLitepp-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ReactiveLitepp
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    add_subdirectory(examples)
endif()

# Option to build tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    enable_testing()
    add_subdirectory(tests)
endif()


