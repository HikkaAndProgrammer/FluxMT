set(PATHS
    "${ROOT}/include/fluxmt/${MODULE}"
    "${ROOT}/include/fluxmt/${MODULE}.hpp"
    "${ROOT}/src/fluxmt/${MODULE}"
)

foreach(PATH IN LISTS PATHS)
    if(IS_DIRECTORY "${PATH}")
        file(GLOB_RECURSE FILES "${PATH}/*.hpp" "${PATH}/*.cpp")
    elseif(EXISTS "${PATH}")
        set(FILES "${PATH}")
    else()
        continue()
    endif()

    foreach(FILE IN LISTS FILES)
        file(STRINGS "${FILE}" HITS REGEX "^[ \t]*#[ \t]*include[ \t]*<${PATTERN}")
        if(HITS)
            message(SEND_ERROR "${FILE}: forbidden include: ${HITS}")
        endif()
    endforeach()
endforeach()
