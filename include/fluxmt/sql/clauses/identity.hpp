#pragma once

// std
#include <string>
#include <utility>

// fluxmt
#include <fluxmt/query/clause.hpp>
#include <fluxmt/sql/clauses/tag.hpp>
#include <fluxmt/sql/slots/from.hpp>
#include <fluxmt/sql/slots/head.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::clauses {
    FLUXMT_EXPORT Clause identifier(slot_ref slot, std::string name, std::string alias = "");

    inline SqlClause<&slots::from, no_needs> from(std::string name, std::string alias = "") {
        return SqlClause<&slots::from, no_needs>(identifier(&slots::from, std::move(name), std::move(alias)));
    }

    inline SqlClause<&slots::insert_target, no_needs> insert_target(std::string name) {
        return SqlClause<&slots::insert_target, no_needs>(identifier(&slots::insert_target, std::move(name)));
    }

    inline SqlClause<&slots::update_target, no_needs> update_target(std::string name) {
        return SqlClause<&slots::update_target, no_needs>(identifier(&slots::update_target, std::move(name)));
    }

    inline SqlClause<&slots::delete_target, no_needs> delete_target(std::string name) {
        return SqlClause<&slots::delete_target, no_needs>(identifier(&slots::delete_target, std::move(name)));
    }

    inline SqlClause<&slots::exists_target, no_needs> exists_target(std::string name) {
        return SqlClause<&slots::exists_target, no_needs>(identifier(&slots::exists_target, std::move(name)));
    }
}
