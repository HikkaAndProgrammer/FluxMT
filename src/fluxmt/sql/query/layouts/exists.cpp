#include "fluxmt/sql/query/layouts/exists.hpp"

// std
#include <utility>

// fluxmt
#include <fluxmt/sql/query/clauses/identity.hpp>
#include <fluxmt/sql/query/kind.hpp>
#include <fluxmt/sql/query/slots/head.hpp>


using namespace flux::impl;


// Implementation --------------------------------------------------------------

Query layouts::exists_in(std::string table) {
    Query q(&kinds::exists);
    q |= clauses::identifier(&slots::exists_target, std::move(table));

    return q;
}
