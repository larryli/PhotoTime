# CMakeLists.txt - Build configuration for tests
#
# Build: mkdir build_tests && cd build_tests
#        cmake .. -G "NMake Makefiles"
#        cmake --build .
#        ctest

cmake_minimum_required(VERSION 3.10)
project(PhotoTimeTests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Test executable
add_executable(test_parsest
    test_parsest.c
    ../src/parsest.c
    ../src/utils.c
)

# Include directories
target_include_directories(test_parsest PRIVATE ../src)

# Compiler options
if(MSVC)
    target_compile_options(test_parsest PRIVATE /utf-8)
endif()

# Preprocessor definitions
target_compile_definitions(test_parsest PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
)

# Enable testing
enable_testing()
add_test(NAME test_parsest COMMAND test_parsest)
