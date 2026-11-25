#pragma once

// ers
#include <erslib/core/type/general.hpp>

// fluxmt
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/query/clause.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    enum class EOrder : ers::u8 { Asc, Desc };
}


namespace flux::impl::clauses {
    FLUXMT_EXPORT Clause order_by(ColumnExpr what, EOrder order = EOrder::Asc);

    FLUXMT_EXPORT Clause order_by_random();
}
