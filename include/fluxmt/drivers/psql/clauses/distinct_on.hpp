#pragma once

// std
#include <concepts>
#include <utility>

// fluxmt
#include <fluxmt/query/slot.hpp>
#include <fluxmt/sql/clauses/column.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/slots/distinct.hpp>


// Slot ------------------------------------------------------------------------

namespace flux::impl::slots {
    inline constexpr slot_t distinct_on {
        .name  = "flux.distinct_on",
        .arity = EArity::Multi,
    };
}


// Layout and clause -----------------------------------------------------------

namespace flux::impl::psql {
    inline constexpr slot_ref distinct_on_excludes[] = { &slots::distinct };

    inline constexpr slot_info_t distinct_on_fragment {
        .slot      = &slots::distinct_on,
        .prefix    = " DISTINCT ON (",
        .separator = ", ",
        .suffix    = ")",
        .excludes  = distinct_on_excludes,
    };


    template<typename... Args>
        requires (sizeof...(Args) > 0 && (std::constructible_from<column_expr_t, Args> && ...))
    SqlClauses<&slots::distinct_on, column_needs<Args...>()> distinct_on(Args&&... args) {
        return SqlClauses<&slots::distinct_on, column_needs<Args...>()>(
            make_columns(&slots::distinct_on, std::forward<Args>(args)...));
    }

    template<IsColumnNamesRange R>
    SqlClauses<&slots::distinct_on, no_needs> distinct_on(R&& names) {
        return SqlClauses<&slots::distinct_on, no_needs>(make_columns(&slots::distinct_on, std::forward<R>(names)));
    }
}
