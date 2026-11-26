#pragma once

// ers
#include <erslib/core/type/general.hpp>

// fluxmt
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/query/clause.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    enum class EOrder : ers::u8 { Asc, Desc };
}


namespace flux::impl::clauses {
    FLUXMT_EXPORT Clause order_by(column_expr_t what, EOrder order = EOrder::Asc);

    FLUXMT_EXPORT Clause order_by_random();
}
