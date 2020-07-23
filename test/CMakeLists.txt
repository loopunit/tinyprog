cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

option(build_tinyprog_test "Build TinyProg tests." ON)
option(build_tinyprog_bench "Build TinyProg benchmark." ON)
option(build_tinyprog_example "Build TinyProg example." ON)
option(build_tinyprog_example2 "Build TinyProg example 2." ON)
option(build_tinyprog_example3 "Build TinyProg example 3." ON)

if (build_tinyprog_test)
	vcpkg_executable(tinyprog_test
	SOURCES 
		test.cpp
	PRIVATE_DEPENDENCIES
		tinyprog
	)
endif()

if (build_tinyprog_bench)
	vcpkg_executable(tinyprog_benchmark
	SOURCES 
		benchmark.cpp
	PRIVATE_DEPENDENCIES
		tinyprog
	)
endif()

if (build_tinyprog_example)
	vcpkg_executable(tinyprog_example
	SOURCES 
		example.cpp
	PRIVATE_DEPENDENCIES
		tinyprog
	)
endif()

if (build_tinyprog_example2)
	vcpkg_executable(tinyprog_example2
	SOURCES 
		example2.cpp
	PRIVATE_DEPENDENCIES
		tinyprog
	)
endif()

if (build_tinyprog_example3)
	vcpkg_executable(tinyprog_example3
	SOURCES 
		example3.cpp
	PRIVATE_DEPENDENCIES
		tinyprog
	)
endif()

if (build_tinyprog_example)
	vcpkg_executable(tinyprog_example_program
	SOURCES 
		example_program.cpp
	PRIVATE_DEPENDENCIES
		tinyprog
	)
endif()

