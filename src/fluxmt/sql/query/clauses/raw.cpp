#include "fluxmt/sql/query/clauses/raw.hpp"

// clauses
#include "raw.inl"


// Implementation --------------------------------------------------------------

flux::impl::Clause flux::impl::make_raw_clause(SlotRef slot, std::string sql, std::vector<Value> values) {
    return make_clause<RawClause>(slot, std::move(sql), std::move(values));
}
