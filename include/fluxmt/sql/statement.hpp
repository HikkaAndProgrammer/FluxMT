#pragma once

// std
#include <string>
#include <vector>

// fluxmt
#include <fluxmt/core/value.hpp>


namespace flux::impl {
    struct statement_t {
        std::string sql;
        std::vector<Value> params;

        bool operator==(const statement_t&) const = default;
    };
}
