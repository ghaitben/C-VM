cmake_minimum_required(VERSION 3.10)

project(Compiler)

add_library(Utils
		tokenizer.h
		tokenizer.c
		)

add_executable(main main.c)
target_link_libraries(main PUBLIC Utils)
