#include "fluxmt/sql/layouts/exists.hpp"

// std
#include <utility>

// fluxmt
#include <fluxmt/sql/clauses/identity.hpp>
#include <fluxmt/sql/slots/head.hpp>


using namespace flux::impl;


// Implementation --------------------------------------------------------------

Query layouts::exists_in(std::string table) {
    Query q(&kinds::exists);
    q |= clauses::identifier(&slots::exists_target, std::move(table));

    return q;
}
