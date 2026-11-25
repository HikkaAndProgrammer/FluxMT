#include "fluxmt/drivers/sqlite/dialect.hpp"

// std
#include <algorithm>
#include <array>
#include <format>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/error.hpp>
#include <fluxmt/sql/query/query.hpp>
#include <fluxmt/sql/query/slots/conflict.hpp>
#include <fluxmt/sql/query/slots/limit.hpp>
#include <fluxmt/sql/query/slots/offset.hpp>


// Details ---------------------------------------------------------------------

namespace {
    // https://sqlite.org/lang_keywords.html
    constexpr auto reserved = std::to_array<std::string_view>({
        "abort", "action", "add", "after", "all", "alter", "always", "analyze", "and", "as", "asc",
        "attach", "autoincrement", "before", "begin", "between", "by", "cascade", "case", "cast", "check",
        "collate", "column", "commit", "conflict", "constraint", "create", "cross", "current",
        "current_date", "current_time", "current_timestamp", "database", "default", "deferrable",
        "deferred", "delete", "desc", "detach", "distinct", "do", "drop", "each", "else", "end", "escape",
        "except", "exclude", "exclusive", "exists", "explain", "fail", "filter", "first", "following",
        "for", "foreign", "from", "full", "generated", "glob", "group", "groups", "having", "if", "ignore",
        "immediate", "in", "index", "indexed", "initially", "inner", "insert", "instead", "intersect",
        "into", "is", "isnull", "join", "key", "last", "left", "like", "limit", "match", "materialized",
        "natural", "no", "not", "nothing", "notnull", "null", "nulls", "of", "offset", "on", "or", "order",
        "others", "outer", "over", "partition", "plan", "pragma", "preceding", "primary", "query", "raise",
        "range", "recursive", "references", "regexp", "reindex", "release", "rename", "replace", "restrict",
        "returning", "right", "rollback", "row", "rows", "savepoint", "select", "set", "table", "temp",
        "temporary", "then", "ties", "to", "transaction", "trigger", "unbounded", "union", "unique",
        "update", "using", "vacuum", "values", "view", "virtual", "when", "where", "window", "with",
        "without",
    });

    static_assert(std::ranges::is_sorted(reserved));
}


// Implementation --------------------------------------------------------------

// sqlite_dialect

flux::impl::sqlite::sqlite_dialect::sqlite_dialect() noexcept :
    Dialect({
        .native_arrays = false,
        .ilike         = false,
        .cast          = ECastStyle::Function,
    }) {
}


std::string_view flux::impl::sqlite::sqlite_dialect::name() const noexcept {
    return "sqlite";
}

std::string flux::impl::sqlite::sqlite_dialect::placeholder(size_t index) const {
    return std::format("?{}", index);
}

bool flux::impl::sqlite::sqlite_dialect::is_reserved(std::string_view word) const noexcept {
    return std::ranges::binary_search(reserved, word);
}


bool flux::impl::sqlite::sqlite_dialect::supports(const Slot* slot) const noexcept {
    return !same_slot(slot, &slots::conflict_constraint);
}

ers::Status flux::impl::sqlite::sqlite_dialect::before_slot(
    Context& ctx, const Query& query, const SlotBinding& binding
) const {
    // OFFSET is only valid after LIMIT, and a negative LIMIT means no limit.
    if (same_slot(binding.slot, &slots::offset) && query.has(&slots::offset) && !query.has(&slots::limit))
        ctx.query += "\nLIMIT -1";

    return ers::ok;
}

ers::Result<flux::impl::function_spelling_t> flux::impl::sqlite::sqlite_dialect::function_spelling(
    std::string_view name, size_t arity
) const {
    if (name == "now") {
        if (arity != 0)
            return ers::make_error(ESqlError::Unsupported, "now() takes no arguments");

        return function_spelling_t { .name = "CURRENT_TIMESTAMP", .call = false };
    }

    if (name == "greatest" || name == "least") {
        if (arity < 2) {
            return ers::make_error(ESqlError::Unsupported,
                "{}() needs two arguments or more, max() and min() of one are aggregates", name);
        }

        return function_spelling_t { .name = name == "greatest" ? "max" : "min" };
    }

    return function_spelling_t {};
}


// Free functions

const flux::impl::sqlite::sqlite_dialect& flux::impl::sqlite::dialect() noexcept {
    static const sqlite_dialect instance;
    return instance;
}

std::span<const std::string_view> flux::impl::sqlite::reserved_words() noexcept {
    return reserved;
}
