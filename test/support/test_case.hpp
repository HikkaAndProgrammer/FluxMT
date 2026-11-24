#pragma once

// std
#include <string>
#include <string_view>


#ifndef TEST_TAGS
#define TEST_TAGS ""
#endif


namespace test {
    const char* reduce_name(std::string_view name);
}


#define FLUXMT_TEST_CASE(NAME, TAGS) \
    TEST_CASE(test::reduce_name(NAME), TEST_TAGS TAGS)
