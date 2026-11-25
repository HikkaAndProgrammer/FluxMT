#include "fluxmt/drivers/psql/dialect.hpp"

// std
#include <algorithm>
#include <array>
#include <format>


// Details ---------------------------------------------------------------------

namespace {
    // pg_get_keywords(), catcode R and T
    constexpr auto reserved = std::to_array<std::string_view>({
        "all", "analyse", "analyze", "and", "any", "array", "as", "asc", "asymmetric", "authorization",
        "binary", "both", "case", "cast", "check", "collate", "collation", "column", "concurrently",
        "constraint", "create", "cross", "current_catalog", "current_date", "current_role", "current_schema",
        "current_time", "current_timestamp", "current_user", "default", "deferrable", "desc", "distinct", "do",
        "else", "end", "except", "false", "fetch", "for", "foreign", "freeze", "from", "full", "grant", "group",
        "having", "ilike", "in", "initially", "inner", "intersect", "into", "is", "isnull", "join", "lateral",
        "leading", "left", "like", "limit", "localtime", "localtimestamp", "natural", "not", "notnull", "null",
        "offset", "on", "only", "or", "order", "outer", "overlaps", "placing", "primary", "references",
        "returning", "right", "select", "session_user", "similar", "some", "symmetric", "system_user", "table",
        "tablesample", "then", "to", "trailing", "true", "union", "unique", "user", "using", "variadic",
        "verbose", "when", "where", "window", "with",
    });

    static_assert(std::ranges::is_sorted(reserved));
}


// Implementation --------------------------------------------------------------

// postgres_dialect

flux::impl::psql::postgres_dialect::postgres_dialect() noexcept :
    Dialect({
        .native_arrays = true,
        .ilike         = true,
        .cast          = ECastStyle::Postfix,
    }) {
}


std::string_view flux::impl::psql::postgres_dialect::name() const noexcept {
    return "postgres";
}

std::string flux::impl::psql::postgres_dialect::placeholder(size_t index) const {
    return std::format("${}", index);
}

bool flux::impl::psql::postgres_dialect::is_reserved(std::string_view word) const noexcept {
    return std::ranges::binary_search(reserved, word);
}


// Free functions

const flux::impl::psql::postgres_dialect& flux::impl::psql::dialect() noexcept {
    static const postgres_dialect instance;
    return instance;
}

std::span<const std::string_view> flux::impl::psql::reserved_words() noexcept {
    return reserved;
}
