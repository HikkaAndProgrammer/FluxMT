include(FetchContent)


# erslib::core from the parent project, an installed package, or fetched by FLUXMT_ERSLIB_TAG.
# Working on both at once: -DFETCHCONTENT_SOURCE_DIR_ERSLIB=<local erslib checkout>.
macro(fluxmt_provide_erslib)
	if(NOT TARGET erslib::core)
		find_package(erslib CONFIG QUIET)
	endif()

	if(TARGET erslib::core)
		message(STATUS "fluxmt: using erslib::core that is already available")
	else()
		message(STATUS "fluxmt: fetching erslib ${FLUXMT_ERSLIB_TAG}")

		set(ERSLIB_ENABLE_REFLECTION ON)
		set(ERSLIB_BUILD_TESTS OFF)
		set(ERSLIB_INSTALL ${FLUXMT_INSTALL})

		FetchContent_Declare(erslib
			GIT_REPOSITORY "${FLUXMT_ERSLIB_REPOSITORY}"
			GIT_TAG "${FLUXMT_ERSLIB_TAG}"
			GIT_SHALLOW TRUE
		)
		FetchContent_MakeAvailable(erslib)
	endif()

	get_target_property(_fluxmt_erslib_defs erslib::core INTERFACE_COMPILE_DEFINITIONS)

	if(NOT "ERSLIB_HAS_REFLECTION" IN_LIST _fluxmt_erslib_defs)
		message(FATAL_ERROR "fluxmt needs erslib built with ERSLIB_ENABLE_REFLECTION=ON.")
	endif()

	unset(_fluxmt_erslib_defs)
endmacro()
