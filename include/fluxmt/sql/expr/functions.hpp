#pragma once

// fluxmt
#include <fluxmt/sql/expr/named.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::clauses {
    inline constexpr named_fn<"coalesce"> coalesce {};
    inline constexpr named_fn<"nullif"> nullif {};
    inline constexpr named_fn<"greatest"> greatest {};
    inline constexpr named_fn<"least"> least {};
    inline constexpr named_fn<"now"> now {};

    inline constexpr binary_op_fn<"||"> concat {};
}
