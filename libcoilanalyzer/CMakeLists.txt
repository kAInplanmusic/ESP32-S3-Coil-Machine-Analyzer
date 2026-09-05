cmake_minimum_required(VERSION 3.15)
project(libcoilanalyzer VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# Options
option(BUILD_SHARED_LIBS "Build as shared library" ON)
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_WASM "Build WebAssembly version" OFF)

# Source files
set(LIBCOILANALYZER_SOURCES
    src/signal_processor.cpp
    src/impact_analyzer.cpp
    src/data_models.cpp
)

set(LIBCOILANALYZER_HEADERS
    include/signal_processor.h
    include/impact_analyzer.h
    include/data_models.h
)

# Main library
add_library(coilanalyzer ${LIBCOILANALYZER_SOURCES} ${LIBCOILANALYZER_HEADERS})

# Include directories
target_include_directories(coilanalyzer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compiler flags
if(MSVC)
    target_compile_options(coilanalyzer PRIVATE /W4 /WX)
else()
    target_compile_options(coilanalyzer PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Optional: WebAssembly build
if(BUILD_WASM)
    add_library(coilanalyzer_wasm ${LIBCOILANALYZER_SOURCES} ${LIBCOILANALYZER_HEADERS})
    target_include_directories(coilanalyzer_wasm
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    set_target_properties(coilanalyzer_wasm PROPERTIES SUFFIX ".a")
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS coilanalyzer
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${LIBCOILANALYZER_HEADERS}
    DESTINATION include/coilanalyzer
)

# Package configuration
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/coilanalyzerConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/coilanalyzerConfigVersion.cmake"
    DESTINATION lib/cmake/coilanalyzer
)

message(STATUS "libcoilanalyzer ${PROJECT_VERSION} configured successfully")
