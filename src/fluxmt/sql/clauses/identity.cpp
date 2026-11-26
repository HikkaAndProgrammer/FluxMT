#include "fluxmt/sql/clauses/identity.hpp"

// fluxmt
#include <fluxmt/sql/slots/from.hpp>

// clauses
#include "identity.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::clauses::identifier(slot_ref slot, std::string name, std::string alias) {
    return make_clause<IdentityClause>(slot, std::move(name), std::move(alias));
}

flux::impl::Clause flux::impl::clauses::from(std::string name, std::string alias) {
    return identifier(&slots::from, std::move(name), std::move(alias));
}
