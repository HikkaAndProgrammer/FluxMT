#pragma once

// std
#include <utility>

// fluxmt
#include <fluxmt/sql/clauses/column.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/slots/returning.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    FLUXMT_EXPORT Clause make_returning_all_clause();
}


namespace flux::impl::clauses {
    template<ColumnItem... Args>
    SqlClauses<&slots::returning, column_needs<Args...>()> returning(Args&&... args) {
        return SqlClauses<&slots::returning, column_needs<Args...>()>(
            make_columns(&slots::returning, std::forward<Args>(args)...));
    }

    template<ColumnNames R>
    SqlClauses<&slots::returning, no_needs> returning(R&& names) {
        return SqlClauses<&slots::returning, no_needs>(make_columns(&slots::returning, std::forward<R>(names)));
    }

    inline SqlClause<&slots::returning, no_needs> returning_all() {
        return SqlClause<&slots::returning, no_needs>(make_returning_all_clause());
    }
}
