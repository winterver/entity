cmake_minimum_required(VERSION 3.15)
project(entity)

add_executable(entity src/entity.c src/lexer.c)
target_compile_options(entity PRIVATE /wd4819)