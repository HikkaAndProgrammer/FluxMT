#pragma once

// std
#include <string_view>


namespace flux::impl {
    struct sql_text {
        std::string_view text;

        consteval sql_text(const char* literal) :
            text(literal) {
        }
    };
}
