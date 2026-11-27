#pragma once

// fluxmt
#include <fluxmt/sql.hpp>

// support
#include "dialect.hpp"


// Backend ---------------------------------------------------------------------

namespace test {
    struct anti_backend {};
}

template<>
struct flux::ext::backend_traits<test::anti_backend> {
    using connection = int;

    static constexpr std::string_view name = "anti";
    static constexpr feature_set features { EFeature::Sql, EFeature::Returning, EFeature::Upsert };

    static constexpr const sql::syntax_t& syntax = test::anti::syntax;

    static const flux::sql::Dialect& dialect() noexcept {
        static const test::AntiDialect instance;
        return instance;
    }
};
