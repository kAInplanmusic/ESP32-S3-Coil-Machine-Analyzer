cmake_minimum_required(VERSION 3.16)
project(coil_analyzer_core LANGUAGES CXX)

add_library(coil_analyzer_core
    src/SignalProcessor.cpp
    src/AdvancedImpactAnalyzer.cpp
    src/CalibrationManager.cpp
)

target_include_directories(coil_analyzer_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(coil_analyzer_core PUBLIC cxx_std_17)

enable_testing()

add_executable(coil_analyzer_core_tests
    tests/test_core.cpp
)

target_link_libraries(coil_analyzer_core_tests PRIVATE coil_analyzer_core)
add_test(NAME coil_analyzer_core_tests COMMAND coil_analyzer_core_tests)
