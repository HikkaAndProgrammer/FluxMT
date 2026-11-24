#----------------------------------------------------------------------------------------------------------------------
# testing framework
#----------------------------------------------------------------------------------------------------------------------


find_package(Catch2 3 CONFIG REQUIRED)
include(Catch)


add_library(test_support
	STATIC
		"support/fake_backend.cpp"
		"support/test_case.cpp"
)
target_include_directories(test_support PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(test_support PUBLIC fluxmt::core)


#----------------------------------------------------------------------------------------------------------------------
# module support: test_support_<module> is linked into every live test of that module
#----------------------------------------------------------------------------------------------------------------------


if(TARGET fluxmt::psql)
    add_library(test_support_psql STATIC "support/psql/live.cpp")
    target_link_libraries(test_support_psql PUBLIC test_support fluxmt::psql Catch2::Catch2)
    target_compile_definitions(test_support_psql PRIVATE
        FLUXMT_TEST_PG_HOST_DEFAULT="${FLUXMT_TEST_PG_HOST}"
        FLUXMT_TEST_PG_PORT_DEFAULT="${FLUXMT_TEST_PG_PORT}"
        FLUXMT_TEST_PG_DB_DEFAULT="${FLUXMT_TEST_PG_DB}"
        FLUXMT_TEST_PG_USER_DEFAULT="${FLUXMT_TEST_PG_USER}"
        FLUXMT_TEST_PG_PASSWORD_DEFAULT="${FLUXMT_TEST_PG_PASSWORD}"
    )
    set_target_properties(test_support_psql PROPERTIES FOLDER "tests/support")
endif()

if(TARGET fluxmt::sqlite)
    add_library(test_support_sqlite STATIC "support/sqlite/db.cpp")
    target_link_libraries(test_support_sqlite PUBLIC test_support fluxmt::sqlite)
    set_target_properties(test_support_sqlite PROPERTIES FOLDER "tests/support")
endif()


#----------------------------------------------------------------------------------------------------------------------
# test groups
#----------------------------------------------------------------------------------------------------------------------


option(FLUXMT_TESTS_PER_CASE "Register every test case as its own ctest entry" OFF)

add_library(test_simple_main OBJECT "simple.cpp")
target_link_libraries(test_simple_main PUBLIC Catch2::Catch2WithMain test_support)

# add_test_group(<kind> <module> SOURCES <...> LIBRARIES <...>) -> executable test_<kind>_<module>
function(add_test_group KIND MODULE)
    cmake_parse_arguments(ARG "" "" "SOURCES;LIBRARIES" ${ARGN})

    set(TARGET "test_${KIND}_${MODULE}")

    add_executable(${TARGET} ${ARG_SOURCES})
    target_link_libraries(${TARGET} PRIVATE test_simple_main ${ARG_LIBRARIES})
    target_compile_definitions(${TARGET} PRIVATE TEST_TAGS="[${KIND}]")
    set_target_properties(${TARGET} PROPERTIES FOLDER "tests/${KIND}")

    set(SERIAL FALSE)
    if(KIND STREQUAL "live")
        set(SERIAL TRUE)
    endif()

    if(FLUXMT_TESTS_PER_CASE)
        catch_discover_tests(${TARGET}
            TEST_PREFIX "${KIND}::${MODULE}::"
            DISCOVERY_MODE PRE_TEST
            ADD_TAGS_AS_LABELS
            PROPERTIES RUN_SERIAL ${SERIAL}
        )
    else()
        add_test(NAME "${KIND}::${MODULE}" COMMAND ${TARGET})
        set_tests_properties("${KIND}::${MODULE}" PROPERTIES
            LABELS "${KIND}"
            RUN_SERIAL ${SERIAL}
            SKIP_RETURN_CODE 4
        )
    endif()

    message(STATUS "Test group: ${KIND}::${MODULE} -> Target: ${TARGET}")
endfunction()


foreach(KIND IN ITEMS simple live)
    set(KIND_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${KIND}")

    add_custom_target(test_${KIND})

    file(GLOB KIND_MODULES CONFIGURE_DEPENDS
        LIST_DIRECTORIES true
        RELATIVE "${KIND_DIR}"
        "${KIND_DIR}/*"
    )

    foreach(MODULE IN LISTS KIND_MODULES)
        if(NOT IS_DIRECTORY "${KIND_DIR}/${MODULE}")
            continue()
        endif()

        if(NOT TARGET fluxmt::${MODULE})
            message(STATUS "Skipping ${KIND} tests for '${MODULE}': fluxmt::${MODULE} is not built")
            continue()
        endif()

        set(GROUP_LIBRARIES fluxmt::${MODULE})

        if(KIND STREQUAL "live")
            if(NOT TARGET test_support_${MODULE})
                message(STATUS "Skipping live tests for '${MODULE}': test_support_${MODULE} is missing")
                continue()
            endif()

            set(GROUP_LIBRARIES test_support_${MODULE})
        endif()

        file(GLOB_RECURSE GROUP_SOURCES CONFIGURE_DEPENDS "${KIND_DIR}/${MODULE}/*.cpp")
        if(NOT GROUP_SOURCES)
            continue()
        endif()

        add_test_group(${KIND} ${MODULE} SOURCES ${GROUP_SOURCES} LIBRARIES ${GROUP_LIBRARIES})
        add_dependencies(test_${KIND} test_${KIND}_${MODULE})
    endforeach()
endforeach()
