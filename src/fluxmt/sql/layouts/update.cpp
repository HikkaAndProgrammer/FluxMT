#include "fluxmt/sql/layouts/update.hpp"

// std
#include <utility>

// fluxmt
#include <fluxmt/sql/clauses/identity.hpp>
#include <fluxmt/sql/kinds.hpp>
#include <fluxmt/sql/slots/head.hpp>


using namespace flux::impl;


// Implementation --------------------------------------------------------------

Query layouts::update(std::string table) {
    Query q(&kinds::update);
    q |= clauses::identifier(&slots::update_target, std::move(table));

    return q;
}
