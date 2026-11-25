#pragma once

// std
#include <concepts>
#include <string>
#include <utility>

// fluxmt
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/query/clause.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    FLUXMT_EXPORT Clause make_join_clause(std::string table, std::string alias, Expr on);
}


namespace flux::impl::clauses {
    // Expr only: 'join("t", "alias")' must not silently bind a string.
    template<std::same_as<Expr> T>
    Clause join(std::string table, std::string alias, T on) {
        return make_join_clause(std::move(table), std::move(alias), std::move(on));
    }

    template<std::same_as<Expr> T>
    Clause join(std::string table, T on) {
        return make_join_clause(std::move(table), std::string(), std::move(on));
    }
}
