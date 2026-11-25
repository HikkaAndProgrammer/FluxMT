#include "fluxmt/sql/query/clauses/identity.hpp"

// fluxmt
#include <fluxmt/sql/query/slots/from.hpp>

// clauses
#include "identity.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::clauses::identifier(SlotRef slot, std::string name, std::string alias) {
    return make_clause<IdentityClause>(slot, std::move(name), std::move(alias));
}

flux::impl::Clause flux::impl::clauses::from(std::string name, std::string alias) {
    return identifier(&slots::from, std::move(name), std::move(alias));
}
