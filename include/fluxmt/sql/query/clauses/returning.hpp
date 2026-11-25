#pragma once

// std
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/sql/query/clauses/column.hpp>
#include <fluxmt/sql/query/slots/returning.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::clauses {
    template<ColumnItem... Args>
    std::vector<Clause> returning(Args&&... args) {
        return make_columns(&slots::returning, std::forward<Args>(args)...);
    }

    template<ColumnNames R>
    std::vector<Clause> returning(R&& names) {
        return make_columns(&slots::returning, std::forward<R>(names));
    }

    FLUXMT_EXPORT Clause returning_all();
}
