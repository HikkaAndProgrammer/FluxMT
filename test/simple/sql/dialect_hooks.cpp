// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <array>
#include <cstddef>
#include <string>
#include <string_view>

// fluxmt
#include <fluxmt/drivers/shared.hpp>
#include <fluxmt/sql.hpp>

// test
#include "support/sql/backend.hpp"
#include "support/test_case.hpp"


using namespace flux::orm;


// Details ---------------------------------------------------------------------

namespace {
    namespace shared = flux::impl::shared;

    inline constexpr flux::ext::named_fn<"forbidden"> forbidden {};


    inline constexpr flux::ext::slot_t truncate_target {
        .name  = "test.truncate_target",
        .arity = flux::ext::EArity::Single,
    };

    constexpr flux::ext::slot_info_t hook_where {
        .slot      = &flux::slots::where,
        .prefix    = "\n/* before where */\nWHERE ",
        .separator = " AND ",
    };

    constexpr flux::ext::slot_info_t hook_limit {
        .slot   = &flux::slots::limit,
        .prefix = "\nFETCH FIRST ",
        .suffix = " ROWS ONLY",
    };

    constexpr flux::ext::slot_info_t select_layout[] = {
        shared::select_keyword,
        shared::select_columns,
        shared::from,
        hook_where,
        shared::order,
        hook_limit,
    };

    constexpr flux::ext::slot_info_t insert_layout[] = {
        shared::insert_into,
        shared::set_as_values,
        shared::insert_columns,
        shared::values,
    };

    constexpr flux::ext::slot_info_t truncate_layout[] = {
        { .slot = &truncate_target, .prefix = "TRUNCATE " },
    };

    struct hook_backend {};


    constexpr flux::sql::function_rule_t now_keyword {
        .name      = "now",
        .max_arity = 0,
        .spelling  = { .text = "CURRENT_TIMESTAMP", .call = false },
    };

    constexpr flux::sql::function_rule_t greatest_max {
        .name     = "greatest",
        .spelling = { .text = "max" },
    };

    constexpr flux::sql::function_rules_t hook_functions { now_keyword, greatest_max };

    // What no rule says
    constexpr flux::sql::function_spelling_t spell_function(std::string_view name, size_t) noexcept {
        if (name == "forbidden")
            return { .supported = false, .reason = "no arguments, one or many" };

        return {};
    }

    constexpr flux::sql::syntax_t hook_syntax {
        .cast      = flux::syntax::cast_function,
        .ilike     = flux::syntax::ilike_via_lower,
        .any       = flux::syntax::any_as_in_list,
        .excluded  = flux::syntax::excluded_table,
        .functions = hook_functions,
        .function  = &spell_function,
    };


    class HookDialect final : public flux::sql::Dialect {
    public:
        HookDialect() noexcept : SqlDialect(hook_syntax) {}

        std::string_view name() const noexcept override { return "hook"; }
        std::string placeholder(size_t index) const override { return "?" + std::to_string(index); }
        bool is_reserved(std::string_view) const noexcept override { return false; }
    };


    const HookDialect& hook() {
        static const HookDialect dialect;
        return dialect;
    }

    ers::Result<flux::sql::statement_t> statement_of(const flux::sql::Query& query) {
        return query.to_statement(hook());
    }

    std::string sql_of(const flux::sql::Query& query) {
        const auto r = statement_of(query);

        return r ? r->text : std::string("<error>");
    }

    template<typename B, const auto& L>
    std::string sql_of(const flux::sql::QueryBuilder<B, L>& builder) {
        return sql_of(builder.erased());
    }

    std::string sql_of(const flux::sql::Expr& expr, const flux::sql::Dialect& dialect = hook()) {
        flux::sql::Context ctx(dialect);

        return expr.render(ctx) ? ctx.text : "<error>";
    }
}

template<>
struct flux::impl::backend_traits<hook_backend> {
    using connection = int;

    static constexpr std::string_view name = "hook";
    static constexpr feature_set features { EFeature::Sql, EFeature::Returning };

    static constexpr const syntax_t& syntax = hook_syntax;

    static const SqlDialect& dialect() noexcept { return hook(); }
};

namespace {
    auto select_from(std::string table, std::string alias = "") {
        return flux::sql::QueryBuilder<hook_backend, select_layout>()
            | from(std::move(table), std::move(alias));
    }

    auto insert_into(std::string table) {
        return flux::sql::QueryBuilder<hook_backend, insert_layout>()
            | insert_target(std::move(table));
    }
}


// Functions -------------------------------------------------------------------

FLUXMT_TEST_CASE("dialect keeps a function name by default", "[sql]") {
    const test::AntiDialect plain;

    CHECK(sql_of(now(), plain) == "now()");
    CHECK(sql_of(greatest(col("a"), col("b")), plain) == "greatest(a, b)");
}

FLUXMT_TEST_CASE("dialect renames a function", "[sql]") {
    CHECK(sql_of(greatest(col("a"), col("b"))) == "max(a, b)");
    CHECK(sql_of(coalesce(col("a"), 0)) == "coalesce(a, ?1)");
}

FLUXMT_TEST_CASE("dialect writes a function as a keyword", "[sql]") {
    CHECK(sql_of(now()) == "CURRENT_TIMESTAMP");
    CHECK(sql_of(col("a") < now()) == "a < CURRENT_TIMESTAMP");
}

FLUXMT_TEST_CASE("dialect can refuse a function", "[sql]") {
    flux::sql::Context ctx(hook());

    const auto r = forbidden(col("a")).render(ctx);

    REQUIRE(!r);
    CHECK(r.error().code() == flux::EQueryError::Unsupported);
}


// Slots -----------------------------------------------------------------------

FLUXMT_TEST_CASE("dialect refuses a slot it does not support", "[sql]") {
    STATIC_REQUIRE(!flux::impl::AcceptsClause<hook_backend, insert_layout, decltype(returning("a"))>);

    const auto refused = statement_of(
        insert_into("t").erased() | columns("a") | values(1) | returning("a"));

    REQUIRE(!refused);
    CHECK(refused.error().code() == flux::EQueryError::ForeignSlot);
    CHECK(refused.error().message().contains("hook"));

    CHECK(sql_of(insert_into("t") | columns("a") | values(1)) == "INSERT INTO t (a)\nVALUES (?1)");
}

FLUXMT_TEST_CASE("dialect owns the text of a slot", "[sql]") {
    CHECK(sql_of(select_from("t") | where(col("a") == 1))
        == "SELECT *\nFROM t\n/* before where */\nWHERE a = ?1");

    CHECK(sql_of(select_from("t")) == "SELECT *\nFROM t");
    CHECK(sql_of(select_from("t") | with_limit(3)) == "SELECT *\nFROM t\nFETCH FIRST ?1 ROWS ONLY");
}

FLUXMT_TEST_CASE("a layout of its own makes a statement of its own", "[sql]") {
    flux::sql::Query q(truncate_layout);
    q |= identifier(&truncate_target, "t");

    CHECK(sql_of(q) == "TRUNCATE t");
}

FLUXMT_TEST_CASE("dialect layout reaches subqueries", "[sql]") {
    CHECK(sql_of(
            select_from("u")
            | where(col("id").in(
                select_from("o")
                | columns("user_id")
                | where(col("x") == 1))))
        == "SELECT *\nFROM u"
           "\n/* before where */"
           "\nWHERE id IN (SELECT user_id\nFROM o\n/* before where */\nWHERE x = ?1)");
}
