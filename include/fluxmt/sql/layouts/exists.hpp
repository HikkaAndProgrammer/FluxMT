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
    inline constexpr statement_kind_t exists { .name = "flux.exists" };
}


namespace flux::impl::layouts {
    FLUXMT_EXPORT Query exists_in(std::string table);
}


namespace flux::impl::builders {
    template<SqlBackend B>
        requires HasKind<B, &kinds::exists>
    QueryBuilder<B, &kinds::exists> exists_in(std::string table) {
        return QueryBuilder<B, &kinds::exists>(layouts::exists_in(std::move(table)));
    }
}
