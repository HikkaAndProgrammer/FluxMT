#include "fluxmt/sql/query/layouts/update.hpp"

// std
#include <utility>

// fluxmt
#include <fluxmt/sql/query/clauses/identity.hpp>
#include <fluxmt/sql/query/kind.hpp>
#include <fluxmt/sql/query/slots/head.hpp>


using namespace flux::impl;


// Implementation --------------------------------------------------------------

Query layouts::update(std::string table) {
    Query q(&kinds::update);
    q |= clauses::identifier(&slots::update_target, std::move(table));

    return q;
}
