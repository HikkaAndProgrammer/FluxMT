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
    inline constexpr statement_kind_t update { .name = "flux.update" };
}


namespace flux::impl::layouts {
    FLUXMT_EXPORT Query update(std::string table);
}


namespace flux::impl::builders {
    template<SqlBackend B>
        requires HasKind<B, &kinds::update>
    QueryBuilder<B, &kinds::update> update(std::string table) {
        return QueryBuilder<B, &kinds::update>(layouts::update(std::move(table)));
    }
}
