#pragma once

// std
#include <string_view>


namespace flux::impl {
    // Only a literal converts, so the text never comes from a runtime string.
    struct sql_text {
        std::string_view text;

        consteval sql_text(const char* literal) :
            text(literal) {
        }
    };
}
