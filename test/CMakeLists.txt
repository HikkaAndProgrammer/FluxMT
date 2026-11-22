#----------------------------------------------------------------------------------------------------------------------
# testing framework
#----------------------------------------------------------------------------------------------------------------------


find_package(Catch2 3 CONFIG REQUIRED)
include(Catch)


add_library(test_support INTERFACE)
target_include_directories(test_support INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")


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
# must-not-compile tests
#----------------------------------------------------------------------------------------------------------------------


# compile_fail/<name>.cpp is built on demand; the test passes only if the error output matches EXPECTED.
function(add_compile_fail_test NAME EXPECTED)
    string(MAKE_C_IDENTIFIER "compile_fail_${NAME}" TARGET)

    add_library(${TARGET} OBJECT EXCLUDE_FROM_ALL "${CMAKE_CURRENT_SOURCE_DIR}/compile_fail/${NAME}.cpp")
    target_link_libraries(${TARGET} PRIVATE fluxmt::core)

    add_test(NAME "compile_fail::${NAME}"
        COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target ${TARGET} --config $<CONFIG>
    )
    set_tests_properties("compile_fail::${NAME}" PROPERTIES PASS_REGULAR_EXPRESSION "${EXPECTED}")
endfunction()

add_compile_fail_test(supports_missing "satisfaction of .*Supports")


#----------------------------------------------------------------------------------------------------------------------
# header self-containment
#----------------------------------------------------------------------------------------------------------------------


# One translation unit per public header, so a header that misses its own includes fails the build.
function(add_header_check MODULE)
    set(HEADERS "fluxmt/${MODULE}.hpp")
    file(GLOB MODULE_HEADERS CONFIGURE_DEPENDS RELATIVE "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/include/fluxmt/${MODULE}/*.hpp"
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

if(TARGET fluxmt::psql)
    add_header_check(psql)
endif()


#----------------------------------------------------------------------------------------------------------------------
# layering
#----------------------------------------------------------------------------------------------------------------------


add_test(NAME layering::core_has_no_pqxx
    COMMAND "${CMAKE_COMMAND}"
        "-DROOT=${PROJECT_SOURCE_DIR}"
        "-DMODULE=core"
        "-DPATTERN=pqxx/"
        -P "${PROJECT_SOURCE_DIR}/cmake/check_no_include.cmake"
)
