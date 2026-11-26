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
struct flux::impl::backend_traits<test::anti_backend> {
    using connection = int;

    static constexpr std::string_view name = "anti";
    static constexpr feature_set features { EFeature::Sql, EFeature::Returning, EFeature::Upsert };

    static constexpr const syntax_t& syntax = test::anti::syntax;

    static constexpr slot_info_view layout(kind_ref kind) noexcept {
        return find_layout(test::anti::layouts, kind);
    }

    static const SqlDialect& dialect() noexcept {
        static const test::AntiDialect instance;
        return instance;
    }
};
