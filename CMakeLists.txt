cmake_minimum_required(VERSION 3.10)

project(Compiler)

add_library(Utils
		tokenizer.h
		tokenizer.c

		error.h
		error.c
		)

add_executable(main main.c)
add_executable(tokenizer_test tokenizer_test.c)

target_link_libraries(main PUBLIC Utils)
