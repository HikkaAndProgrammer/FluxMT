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
    inline constexpr statement_kind_t erase { .name = "flux.delete" };
}


namespace flux::impl::layouts {
    FLUXMT_EXPORT Query delete_from(std::string table);
}


namespace flux::impl::builders {
    template<SqlBackend B>
        requires HasKind<B, &kinds::erase>
    QueryBuilder<B, &kinds::erase> delete_from(std::string table) {
        return QueryBuilder<B, &kinds::erase>(layouts::delete_from(std::move(table)));
    }
}
