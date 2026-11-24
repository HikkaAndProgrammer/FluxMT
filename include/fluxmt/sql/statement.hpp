#pragma once

// std
#include <string>
#include <vector>

// fluxmt
#include <fluxmt/core/value.hpp>


namespace flux::impl {
    struct Statement {
        std::string sql;
        std::vector<Value> params;

        bool operator==(const Statement&) const = default;
    };
}
