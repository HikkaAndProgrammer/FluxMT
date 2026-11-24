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
    add_library(test_support_psql STATIC "support/drivers/psql/live.cpp")
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

if(TARGET fluxmt::redis)
    add_library(test_support_redis STATIC "support/drivers/redis/live.cpp")
    target_link_libraries(test_support_redis PUBLIC test_support fluxmt::redis Catch2::Catch2)
    target_compile_definitions(test_support_redis PRIVATE
        FLUXMT_TEST_REDIS_HOST_DEFAULT="${FLUXMT_TEST_REDIS_HOST}"
        FLUXMT_TEST_REDIS_PORT_DEFAULT="${FLUXMT_TEST_REDIS_PORT}"
        FLUXMT_TEST_REDIS_DB_DEFAULT="${FLUXMT_TEST_REDIS_DB}"
    )
    set_target_properties(test_support_redis PROPERTIES FOLDER "tests/support")
endif()

if(TARGET fluxmt::sqlite)
    add_library(test_support_sqlite STATIC "support/drivers/sqlite/db.cpp")
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

    # core, sql, ... lie directly in the kind directory, drivers in drivers/<name>
    file(GLOB KIND_GROUPS CONFIGURE_DEPENDS
        LIST_DIRECTORIES true
        RELATIVE "${KIND_DIR}"
        "${KIND_DIR}/*"
    )
    list(REMOVE_ITEM KIND_GROUPS "drivers")

    file(GLOB DRIVER_GROUPS CONFIGURE_DEPENDS
        LIST_DIRECTORIES true
        RELATIVE "${KIND_DIR}"
        "${KIND_DIR}/drivers/*"
    )

    foreach(GROUP_PATH IN LISTS KIND_GROUPS DRIVER_GROUPS)
        if(NOT IS_DIRECTORY "${KIND_DIR}/${GROUP_PATH}")
            continue()
        endif()

        cmake_path(GET GROUP_PATH FILENAME MODULE)

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

        file(GLOB_RECURSE GROUP_SOURCES CONFIGURE_DEPENDS "${KIND_DIR}/${GROUP_PATH}/*.cpp")
        if(NOT GROUP_SOURCES)
            continue()
        endif()

        add_test_group(${KIND} ${MODULE} SOURCES ${GROUP_SOURCES} LIBRARIES ${GROUP_LIBRARIES})
        add_dependencies(test_${KIND} test_${KIND}_${MODULE})
    endforeach()
endforeach()


#----------------------------------------------------------------------------------------------------------------------
# header self-containment
#----------------------------------------------------------------------------------------------------------------------


# One translation unit per public header, so a header that misses its own includes fails the build.
function(add_header_check MODULE)
    set(HEADERS "fluxmt/${MODULE}.hpp")
    file(GLOB MODULE_HEADERS CONFIGURE_DEPENDS RELATIVE "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/include/fluxmt/${MODULE}/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/fluxmt/drivers/${MODULE}/*.hpp"
    )
    list(APPEND HEADERS ${MODULE_HEADERS})

    set(SOURCES "")
    foreach(HEADER IN LISTS HEADERS)
        string(MAKE_C_IDENTIFIER "${HEADER}" ID)
        set(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/header_check/${ID}.cpp")
        file(CONFIGURE OUTPUT "${SOURCE}" CONTENT "#include <@HEADER@>\n" @ONLY)
        list(APPEND SOURCES "${SOURCE}")
    endforeach()

    add_library(header_check_${MODULE} OBJECT ${SOURCES})
    target_link_libraries(header_check_${MODULE} PRIVATE fluxmt::${MODULE})
endfunction()


add_header_check(core)
add_header_check(sql)

if(TARGET fluxmt::psql)
    add_header_check(psql)
endif()

if(TARGET fluxmt::sqlite)
    add_header_check(sqlite)
endif()

if(TARGET fluxmt::redis)
    add_header_check(redis)
endif()


#----------------------------------------------------------------------------------------------------------------------
# layering
#----------------------------------------------------------------------------------------------------------------------


function(add_layering_check MODULE FORBIDDEN PATTERN)
    add_test(NAME layering::${MODULE}_has_no_${FORBIDDEN}
        COMMAND "${CMAKE_COMMAND}"
            "-DROOT=${PROJECT_SOURCE_DIR}"
            "-DMODULE=${MODULE}"
            "-DPATTERN=${PATTERN}"
            -P "${PROJECT_SOURCE_DIR}/cmake/check_no_include.cmake"
    )
endfunction()


add_layering_check(core pqxx "pqxx/")
add_layering_check(core sqlite3 "sqlite3")
add_layering_check(core hiredis "hiredis")
add_layering_check(core sql "fluxmt/sql")

add_layering_check(sql pqxx "pqxx/")
add_layering_check(sql sqlite3 "sqlite3")
add_layering_check(sql hiredis "hiredis")
