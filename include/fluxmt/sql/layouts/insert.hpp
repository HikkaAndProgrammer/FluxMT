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
    inline constexpr statement_kind_t insert { .name = "flux.insert" };
}


namespace flux::impl::layouts {
    FLUXMT_EXPORT Query insert_into(std::string table);
}


namespace flux::impl::builders {
    template<SqlBackend B>
        requires HasKind<B, &kinds::insert>
    QueryBuilder<B, &kinds::insert> insert_into(std::string table) {
        return QueryBuilder<B, &kinds::insert>(layouts::insert_into(std::move(table)));
    }
}
