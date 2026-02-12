cmake_minimum_required(VERSION 3.10)

project(chip8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)

add_executable(chip8 src/main.c)
