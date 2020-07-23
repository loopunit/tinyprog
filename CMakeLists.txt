cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(tinyexprpp VERSION 0.0 LANGUAGES CXX)

include(FetchContent)

set(VCPKG_DEVELOP_TRIPLET x64-windows-static-md)

macro(vcpkg_setup_ez)
	cmake_parse_arguments(
		"_arg"
		""
		"REPO;TAG;DIR;UPDATE_DISCONNECTED"
		""
		${ARGN}
	)

	if(NOT _arg_TAG)
		set(vcpkg_ez_SOURCE_DIR ${_arg_DIR})
	else()
		FetchContent_Declare(
			vcpkg_ez
			GIT_REPOSITORY		${_arg_REPO}
			GIT_TAG				${_arg_TAG}
			SOURCE_DIR			${_arg_DIR}
			UPDATE_DISCONNECTED	${_arg_UPDATE_DISCONNECTED}
		)

		FetchContent_GetProperties(vcpkg_ez)

		if(NOT vcpkg_ez_POPULATED)
			FetchContent_Populate(vcpkg_ez)
		endif()
	endif()
endmacro()

if(NOT VCPKG_DEVELOP_ROOT_DIR)
	set(VCPKG_DEVELOP_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/vcpkg_root)
endif()

if(VCPKG_DEVELOP_EZ_DIR)
	vcpkg_setup_ez(DIR ${VCPKG_DEVELOP_EZ_DIR})
else()
	vcpkg_setup_ez(
		REPO https://github.com/loopunit/vcpkg_ez.git
		TAG HEAD
		DIR ${VCPKG_DEVELOP_ROOT_DIR}/vcpkg_ez
	)
endif()

include(${vcpkg_ez_SOURCE_DIR}/vcpkg_ez.cmake)

option(TE_POW_FROM_RIGHT "Evaluate exponents from right to left." OFF)
option(TE_NAT_LOG "Define the log function as natural logarithm." OFF)

vcpkg_interface_library(${PROJECT_NAME}
	INTERFACE_HEADERS
		include/tinyexpr.h
	INTERFACE_INCLUDES
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(${PROJECT_NAME}
	INTERFACE
		$<IF:$<BOOL:${TE_POW_FROM_RIGHT}>,"TE_POW_FROM_RIGHT",>
		$<IF:$<BOOL:${TE_NAT_LOG}>,"TE_NAT_LOG",>
)

if(ENABLE_TESTING)
	add_subdirectory(test)
endif()