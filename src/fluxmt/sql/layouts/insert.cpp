#include "fluxmt/sql/layouts/insert.hpp"

// std
#include <utility>

// fluxmt
#include <fluxmt/sql/clauses/identity.hpp>
#include <fluxmt/sql/slots/head.hpp>


using namespace flux::impl;


// Implementation --------------------------------------------------------------

Query layouts::insert_into(std::string table) {
    Query q(&kinds::insert);
    q |= clauses::identifier(&slots::insert_target, std::move(table));

    return q;
}
