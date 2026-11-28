#pragma once

// std
#include <string>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/core/value.hpp>
#include <fluxmt/core/value_traits.hpp>
#include <fluxmt/query/clause.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    FLUXMT_EXPORT Clause make_raw_clause(slot_ref slot, std::string sql, std::vector<Value> values);
}


namespace flux::impl::clauses {
    template<typename... Args>
    Clause raw(slot_ref slot, std::string sql, Args&&... args) {
        std::vector<Value> values;
        values.reserve(sizeof...(Args));
        (values.push_back(to_value(std::forward<Args>(args))), ...);

        return make_raw_clause(slot, std::move(sql), std::move(values));
    }
}
