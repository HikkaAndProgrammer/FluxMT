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
# stateless tests
#----------------------------------------------------------------------------------------------------------------------


add_library(test_simple_main OBJECT "simple.cpp")
target_link_libraries(test_simple_main PUBLIC Catch2::Catch2WithMain test_support)

set(SIMPLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/simple")

file(GLOB SIMPLE_MODULES CONFIGURE_DEPENDS
    LIST_DIRECTORIES true
    RELATIVE "${SIMPLE_DIR}"
    "${SIMPLE_DIR}/*"
)

add_custom_target(test_simple)

# simple/<module>/sub/name.cpp -> test_simple_<module>_sub_name, linked with fluxmt::<module>
function(add_simple_test TEST_NAME)
    string(REGEX MATCH "^[^/]+" MODULE "${TEST_NAME}")
    string(MAKE_C_IDENTIFIER "test_simple_${TEST_NAME}" TEST_TARGET)

    add_executable(${TEST_TARGET} "${SIMPLE_DIR}/${TEST_NAME}.cpp")
    target_link_libraries(${TEST_TARGET} PRIVATE test_simple_main fluxmt::${MODULE})
    set_target_properties(${TEST_TARGET} PROPERTIES FOLDER "tests/simple/${MODULE}")
    target_compile_definitions(${TEST_TARGET} PRIVATE TEST_TAGS="[simple]")

    catch_discover_tests(${TEST_TARGET} TEST_PREFIX "${TEST_NAME}::" DISCOVERY_MODE PRE_TEST ADD_TAGS_AS_LABELS)
    add_dependencies(test_simple_${MODULE} ${TEST_TARGET})

    message(STATUS "Discovered simple test: ${TEST_NAME} -> Target: ${TEST_TARGET}")
endfunction()

foreach(MODULE IN LISTS SIMPLE_MODULES)
    if(NOT IS_DIRECTORY "${SIMPLE_DIR}/${MODULE}")
        continue()
    endif()

    if(NOT TARGET fluxmt::${MODULE})
        message(STATUS "Skipping simple tests for '${MODULE}': fluxmt::${MODULE} is not built")
        continue()
    endif()

    add_custom_target(test_simple_${MODULE})
    add_dependencies(test_simple test_simple_${MODULE})

    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${SIMPLE_DIR}/${MODULE}/*.cpp")

    foreach(TEST_SOURCE IN LISTS TEST_SOURCES)
        file(RELATIVE_PATH TEST_NAME "${SIMPLE_DIR}" "${TEST_SOURCE}")
        string(REGEX REPLACE "\\.cpp$" "" TEST_NAME "${TEST_NAME}")

        add_simple_test(${TEST_NAME})
    endforeach()
endforeach()


#----------------------------------------------------------------------------------------------------------------------
# live tests
#----------------------------------------------------------------------------------------------------------------------


set(LIVE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/live")

file(GLOB LIVE_MODULES CONFIGURE_DEPENDS
    LIST_DIRECTORIES true
    RELATIVE "${LIVE_DIR}"
    "${LIVE_DIR}/*"
)

add_custom_target(test_live)

# live/<module>/sub/name.cpp -> test_live_<module>_sub_name, linked with test_support_<module>
foreach(MODULE IN LISTS LIVE_MODULES)
    if(NOT IS_DIRECTORY "${LIVE_DIR}/${MODULE}")
        continue()
    endif()

    if(NOT TARGET fluxmt::${MODULE} OR NOT TARGET test_support_${MODULE})
        message(STATUS "Skipping live tests for '${MODULE}': fluxmt::${MODULE} or test_support_${MODULE} is missing")
        continue()
    endif()

    file(GLOB_RECURSE LIVE_SOURCES CONFIGURE_DEPENDS "${LIVE_DIR}/${MODULE}/*.cpp")

    foreach(SOURCE IN LISTS LIVE_SOURCES)
        file(RELATIVE_PATH NAME "${LIVE_DIR}" "${SOURCE}")
        string(REGEX REPLACE "\\.cpp$" "" NAME "${NAME}")
        string(MAKE_C_IDENTIFIER "test_live_${NAME}" TARGET)

        add_executable(${TARGET} "${SOURCE}")
        target_link_libraries(${TARGET} PRIVATE test_simple_main test_support_${MODULE})
        target_compile_definitions(${TARGET} PRIVATE TEST_TAGS="[live]")
        set_target_properties(${TARGET} PROPERTIES FOLDER "tests/live")

        catch_discover_tests(${TARGET}
            TEST_PREFIX "live::${NAME}::"
            DISCOVERY_MODE PRE_TEST
            ADD_TAGS_AS_LABELS
            PROPERTIES RUN_SERIAL TRUE
        )
        add_dependencies(test_live ${TARGET})
    endforeach()
endforeach()
