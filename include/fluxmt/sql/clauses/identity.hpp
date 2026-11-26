#pragma once

// std
#include <string>
#include <utility>

// fluxmt
#include <fluxmt/query/clause.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/slots/from.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::clauses {
    FLUXMT_EXPORT Clause identifier(slot_ref slot, std::string name, std::string alias = "");

    inline SqlClause<&slots::from, no_needs> from(std::string name, std::string alias = "") {
        return SqlClause<&slots::from, no_needs>(identifier(&slots::from, std::move(name), std::move(alias)));
    }
}
