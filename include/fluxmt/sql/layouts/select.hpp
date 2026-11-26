#pragma once

// std
#include <string>
#include <utility>

// fluxmt
#include <fluxmt/query/kind.hpp>
#include <fluxmt/query/query.hpp>
#include <fluxmt/sql/builder.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::kinds {
    inline constexpr statement_kind_t select { .name = "flux.select" };
}


namespace flux::impl::layouts {
    FLUXMT_EXPORT Query select_from(std::string table, std::string alias = "");

    FLUXMT_EXPORT Query select();
}


namespace flux::impl::builders {
    template<SqlBackend B>
        requires HasKind<B, &kinds::select>
    QueryBuilder<B, &kinds::select> select_from(std::string table, std::string alias = "") {
        return QueryBuilder<B, &kinds::select>(layouts::select_from(std::move(table), std::move(alias)));
    }

    template<SqlBackend B>
        requires HasKind<B, &kinds::select>
    QueryBuilder<B, &kinds::select> select() {
        return QueryBuilder<B, &kinds::select>(layouts::select());
    }
}
