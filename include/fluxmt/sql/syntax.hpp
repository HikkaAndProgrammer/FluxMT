#pragma once

// std
#include <string_view>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/dialect.hpp>

// export
#include <fluxmt/export.hpp>


// Strategies for syntax_t: a dialect picks the ones that match its SQL, none of them is a default.
namespace flux::impl::syntax {
    // x::type
    FLUXMT_EXPORT ers::Status cast_postfix(Context& ctx, const Expr& operand, std::string_view type);

    // CAST(x AS type)
    FLUXMT_EXPORT ers::Status cast_function(Context& ctx, const Expr& operand, std::string_view type);

    // a ILIKE b
    FLUXMT_EXPORT ers::Status ilike_operator(Context& ctx, const Expr& lhs, const Expr& rhs);

    // lower(a) LIKE lower(b)
    FLUXMT_EXPORT ers::Status ilike_via_lower(Context& ctx, const Expr& lhs, const Expr& rhs);

    // x = ANY(array), the array is one parameter
    FLUXMT_EXPORT ers::Status any_native(Context& ctx, const Expr& operand, const Expr& array);

    // x IN (a, b, c), a bound array is spread into parameters
    FLUXMT_EXPORT ers::Status any_as_in_list(Context& ctx, const Expr& operand, const Expr& array);

    // excluded.column
    FLUXMT_EXPORT ers::Status excluded_table(Context& ctx, std::string_view column);
}
