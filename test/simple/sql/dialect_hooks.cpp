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
#include "support/sql/dialect.hpp"
#include "support/test_case.hpp"


using namespace flux::orm;


// Details ---------------------------------------------------------------------

namespace {
    namespace shared = flux::impl::shared;

    inline constexpr flux::ext::named_fn<"forbidden"> forbidden {};


    inline constexpr flux::ext::statement_kind_t truncate_kind { .name = "test.truncate" };

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

    constexpr auto layouts = std::to_array<flux::sql::layout_entry_t>({
        { .kind = &flux::kinds::select, .layout = select_layout },
        { .kind = &flux::kinds::insert, .layout = insert_layout },
        { .kind = &truncate_kind, .layout = truncate_layout },
    });


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

        flux::ext::slot_info_view layout(flux::ext::kind_ref kind) const noexcept override {
            return flux::ext::find_layout(layouts, kind);
        }
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

    std::string sql_of(const flux::sql::Expr& expr, const flux::sql::Dialect& dialect = hook()) {
        flux::sql::Context ctx(dialect);

        return expr.render(ctx) ? ctx.text : "<error>";
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
    const auto refused = statement_of(insert_into("t") | columns("a") | values(1) | returning("a"));

    REQUIRE(!refused);
    CHECK(refused.error().code() == flux::EQueryError::Unsupported);
    CHECK(refused.error().message().contains("hook"));

    CHECK(sql_of(insert_into("t") | columns("a") | values(1)) == "INSERT INTO t (a)\nVALUES (?1)");
}

FLUXMT_TEST_CASE("dialect owns the text of a slot", "[sql]") {
    CHECK(sql_of(select_from("t") | where(col("a") == 1))
        == "SELECT *\nFROM t\n/* before where */\nWHERE a = ?1");

    CHECK(sql_of(select_from("t")) == "SELECT *\nFROM t");
    CHECK(sql_of(select_from("t") | with_limit(3)) == "SELECT *\nFROM t\nFETCH FIRST ?1 ROWS ONLY");
}

FLUXMT_TEST_CASE("dialect refuses a statement it has no layout for", "[sql]") {
    const auto refused = statement_of(delete_from("t"));

    REQUIRE(!refused);
    CHECK(refused.error().code() == flux::EQueryError::Unsupported);
    CHECK(refused.error().message().contains("hook"));
}

FLUXMT_TEST_CASE("dialect can have a statement of its own kind", "[sql]") {
    flux::sql::Query q(&truncate_kind);
    q |= identifier(&truncate_target, "t");

    CHECK(sql_of(q) == "TRUNCATE t");

    const test::AntiDialect other;
    const auto refused = q.to_statement(other);

    REQUIRE(!refused);
    CHECK(refused.error().code() == flux::EQueryError::Unsupported);
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
