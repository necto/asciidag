cmake_minimum_required(VERSION 3.13)

project(gtest-demo LANGUAGES CXX)

# set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Workaround to see stdlib debug symbols,
# needed for pretty printing standard containers in GDB
add_compile_options(-D_GLIBCXX_DEBUG)

add_subdirectory(src)

enable_testing()

add_subdirectory(test)
