cmake_minimum_required(VERSION 3.28)

project(fluxmt
	VERSION 0.1.0
	DESCRIPTION "Multi-target database layer: one core, a driver per database"
	LANGUAGES CXX
)


#----------------------------------------------------------------------------------------------------------------------
# general settings and options
#----------------------------------------------------------------------------------------------------------------------


include(cmake/utils.cmake)
include(GNUInstallDirs)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" IS_TOP_LEVEL)


option(FLUXMT_SHARED_LIBS "Build as .so" OFF)
option(FLUXMT_BUILD_TESTS "Build fluxmt tests" OFF)
option(FLUXMT_INSTALL "Generate target for installing fluxmt" ${IS_TOP_LEVEL})

option(FLUXMT_BUILD_PSQL "Build fluxmt::psql (PostgreSQL through libpqxx)" OFF)


set(FLUXMT_ERSLIB_REPOSITORY "https://github.com/HikkaAndProgrammer/erslib.git"
	CACHE STRING "Where to fetch erslib from when find_package can't find it"
)
set(FLUXMT_ERSLIB_TAG "v1.2.13"
	CACHE STRING "erslib branch, tag or commit to fetch"
)

set_if_undefined(FLUXMT_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/fluxmt"
	CACHE STRING "Install path for fluxmt package-related CMake files"
)

if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


#----------------------------------------------------------------------------------------------------------------------
# dependencies
#----------------------------------------------------------------------------------------------------------------------


include(cmake/reflection.cmake)
include(cmake/erslib.cmake)

fluxmt_require_reflection()
fluxmt_provide_erslib()


#----------------------------------------------------------------------------------------------------------------------
# helper functions
#----------------------------------------------------------------------------------------------------------------------


function(fluxmt_add_library target_name alias_name)
	if(FLUXMT_SHARED_LIBS)
		add_library(${target_name} SHARED)
	else()
		add_library(${target_name} STATIC)
	endif()

	add_library(fluxmt::${alias_name} ALIAS ${target_name})

	set_target_properties(${target_name} PROPERTIES EXPORT_NAME ${alias_name})

	target_compile_features(${target_name} PUBLIC cxx_std_26)

	target_include_directories(${target_name}
		PUBLIC
			"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
			"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
	)

	target_compile_definitions(${target_name}
		PUBLIC
			"$<$<NOT:$<BOOL:${FLUXMT_SHARED_LIBS}>>:FLUXMT_STATIC_DEFINE>"
	)

	message(STATUS "fluxmt: added target ${target_name} with alias ${alias_name}")
endfunction()


# A driver is fluxmt::<name>: fluxmt::core plus the client library of one database.
function(fluxmt_add_driver name)
	cmake_parse_arguments(ARG "" "" "SOURCES" ${ARGN})

	fluxmt_add_library(fluxmt_${name} ${name})

	target_sources(fluxmt_${name} PRIVATE ${ARG_SOURCES})
	target_link_libraries(fluxmt_${name} PUBLIC fluxmt_core)

	set(FLUXMT_TARGETS ${FLUXMT_TARGETS} fluxmt_${name} PARENT_SCOPE)
	set(FLUXMT_DRIVERS ${FLUXMT_DRIVERS} ${name} PARENT_SCOPE)
endfunction()


function(fluxmt_required_package target_name package_name dep_target_name)
	find_package(${package_name} CONFIG REQUIRED)
	target_link_libraries(${target_name} PUBLIC ${dep_target_name})
endfunction()


set(FLUXMT_TARGETS "")
set(FLUXMT_DRIVERS "")


#----------------------------------------------------------------------------------------------------------------------
# fluxmt::core
#----------------------------------------------------------------------------------------------------------------------


fluxmt_add_library(fluxmt_core core)

target_sources(fluxmt_core
	PRIVATE
		"include/fluxmt/export.hpp"

		"src/fluxmt/core/version.cpp"
)

target_link_libraries(fluxmt_core
	PUBLIC
		erslib::core
)

target_compile_options(fluxmt_core
	PUBLIC
		$<$<COMPILE_LANG_AND_ID:CXX,GNU>:-freflection>
)

configure_file(cmake/version.hpp.in include/fluxmt/version.hpp @ONLY)

list(APPEND FLUXMT_TARGETS fluxmt_core)


#----------------------------------------------------------------------------------------------------------------------
# fluxmt::psql
#----------------------------------------------------------------------------------------------------------------------


if(FLUXMT_BUILD_PSQL)
	fluxmt_add_driver(psql
		SOURCES
			"src/fluxmt/psql/options.cpp"
	)

	find_package(PostgreSQL)
	fluxmt_required_package(fluxmt_psql libpqxx libpqxx::pqxx)
endif()


#----------------------------------------------------------------------------------------------------------------------
# export header
#----------------------------------------------------------------------------------------------------------------------


include(GenerateExportHeader)
set(EXPORT_FILE_NAME "export_shared.hpp")

if(NOT FLUXMT_SHARED_LIBS)
	set(EXPORT_FILE_NAME "export_static.hpp")
endif()

generate_export_header(fluxmt_core
	BASE_NAME fluxmt
	EXPORT_FILE_NAME include/fluxmt/${EXPORT_FILE_NAME}
)


#----------------------------------------------------------------------------------------------------------------------
# install
#----------------------------------------------------------------------------------------------------------------------


if(FLUXMT_INSTALL AND NOT CMAKE_SKIP_INSTALL_RULES)
	include(CMakePackageConfigHelpers)

	configure_package_config_file(
		cmake/fluxmt-config.cmake.in
		fluxmt-config.cmake
		INSTALL_DESTINATION "${FLUXMT_INSTALL_CMAKEDIR}"
	)

	write_basic_package_version_file(
		fluxmt-config-version.cmake
		COMPATIBILITY SameMinorVersion
	)

	install(TARGETS ${FLUXMT_TARGETS} EXPORT fluxmt_export
		RUNTIME COMPONENT fluxmt
		LIBRARY COMPONENT fluxmt NAMELINK_COMPONENT fluxmt_dev
		ARCHIVE COMPONENT fluxmt_dev
		INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
	)

	install(
		DIRECTORY include/
		TYPE INCLUDE
		COMPONENT fluxmt_dev
		PATTERN "export_static.hpp" EXCLUDE
		PATTERN "export_shared.hpp" EXCLUDE
	)

	install(
		FILES
			"${CMAKE_CURRENT_BINARY_DIR}/include/fluxmt/${EXPORT_FILE_NAME}"
			"${CMAKE_CURRENT_BINARY_DIR}/include/fluxmt/version.hpp"
		COMPONENT fluxmt_dev
		DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fluxmt"
	)

	if(FLUXMT_SHARED_LIBS)
		set(TARGETS_FILE "fluxmt-shared-targets.cmake")
	else()
		set(TARGETS_FILE "fluxmt-static-targets.cmake")
	endif()

	install(EXPORT fluxmt_export
		COMPONENT fluxmt_dev
		FILE "${TARGETS_FILE}"
		DESTINATION "${FLUXMT_INSTALL_CMAKEDIR}"
		NAMESPACE fluxmt::
	)

	install(
		FILES
			"${CMAKE_CURRENT_BINARY_DIR}/fluxmt-config.cmake"
			"${CMAKE_CURRENT_BINARY_DIR}/fluxmt-config-version.cmake"
		COMPONENT fluxmt_dev
		DESTINATION "${FLUXMT_INSTALL_CMAKEDIR}"
	)
endif()


#----------------------------------------------------------------------------------------------------------------------
# other targets
#----------------------------------------------------------------------------------------------------------------------


if(FLUXMT_BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()
