cmake_minimum_required(VERSION 3.10)

add_compile_options(-g)

project(Compiler)

add_library(Utils
		tokenizer.h
		tokenizer.c

		error.h
		error.c

		parser.h
		parser.c

		vm.h
		vm.c
		
		value.h
		value.c

		debug.h
		debug.c

		hash_table.h
		hash_table.c
		)

add_executable(main main.c)
add_executable(tokenizer_test tokenizer_test.c)
add_executable(hash_table_test hash_table_test.c)

target_link_libraries(main PUBLIC Utils)
target_link_libraries(hash_table_test PUBLIC Utils)
