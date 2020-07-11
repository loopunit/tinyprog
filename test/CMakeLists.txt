cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

option(build_tinyexpr_test "Build TinyExpr tests." ON)
option(build_tinyexpr_bench "Build TinyExpr benchmark." ON)
option(build_tinyexpr_example "Build TinyExpr example." ON)
option(build_tinyexpr_example2 "Build TinyExpr example 2." ON)
option(build_tinyexpr_example3 "Build TinyExpr example 3." ON)

if (build_tinyexpr_test)
	vcpkg_executable(tinyexpr_test
	SOURCES 
		test.c
	PRIVATE_DEPENDENCIES
		tinyexprpp
	)
endif()

if (build_tinyexpr_bench)
	vcpkg_executable(tinyexpr_benchmark
	SOURCES 
		benchmark.c
	PRIVATE_DEPENDENCIES
		tinyexprpp
	)
endif()

if (build_tinyexpr_example)
	vcpkg_executable(tinyexpr_example
	SOURCES 
		example.c
	PRIVATE_DEPENDENCIES
		tinyexprpp
	)
endif()

if (build_tinyexpr_example2)
	vcpkg_executable(tinyexpr_example2
	SOURCES 
		example2.c
	PRIVATE_DEPENDENCIES
		tinyexprpp
	)
endif()

if (build_tinyexpr_example3)
	vcpkg_executable(tinyexpr_example3
	SOURCES 
		example3.c
	PRIVATE_DEPENDENCIES
		tinyexprpp
	)
endif()
