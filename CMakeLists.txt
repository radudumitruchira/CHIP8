cmake_minimum_required(VERSION 3.10)

project(chip8)

add_subdirectory(vendor/SDL2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)

set(CMAKE_GENERATOR_PLATFORM x64)

add_executable(chip8 src/main.c)
target_link_libraries(chip8 SDL2)
target_include_directories(chip8 PRIVATE vendor/SDL2/include)
