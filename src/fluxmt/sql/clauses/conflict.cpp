#include "fluxmt/sql/clauses/conflict.hpp"

// fluxmt
#include <fluxmt/sql/clauses/raw.hpp>

// clauses
#include "conflict_update.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::make_conflict_update_clause(std::vector<std::string> columns) {
    return make_clause<ConflictUpdateClause>(std::move(columns));
}


// Shortcuts

flux::impl::Clause flux::impl::make_do_nothing_clause() {
    return make_raw_clause(&slots::conflict_action, "DO NOTHING", {});
}
