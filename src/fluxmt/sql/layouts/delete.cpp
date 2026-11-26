#include "fluxmt/sql/layouts/delete.hpp"

// std
#include <utility>

// fluxmt
#include <fluxmt/sql/clauses/identity.hpp>
#include <fluxmt/sql/slots/head.hpp>


using namespace flux::impl;


// Implementation --------------------------------------------------------------

Query layouts::delete_from(std::string table) {
    Query q(&kinds::erase);
    q |= clauses::identifier(&slots::delete_target, std::move(table));

    return q;
}
