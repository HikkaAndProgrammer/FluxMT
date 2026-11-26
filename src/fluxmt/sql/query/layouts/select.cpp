#include "fluxmt/sql/query/layouts/select.hpp"

// std
#include <utility>

// fluxmt
#include <fluxmt/sql/query/clauses/identity.hpp>
#include <fluxmt/sql/query/kind.hpp>


using namespace flux::impl;


// Implementation --------------------------------------------------------------

Query layouts::select_from(std::string table, std::string alias) {
    Query q(&kinds::select);
    q |= clauses::from(std::move(table), std::move(alias));

    return q;
}

Query layouts::select() {
    return Query(&kinds::select);
}
