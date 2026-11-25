// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <cstddef>
#include <string>
#include <string_view>

// fluxmt
#include <fluxmt/sql.hpp>

// test
#include "support/sql/dialect.hpp"
#include "support/test_case.hpp"


using namespace flux::orm;


// Details ---------------------------------------------------------------------

namespace {
    inline constexpr flux::ext::fn_t<"forbidden"> forbidden {};


    // Refuses RETURNING, writes a comment before WHERE and spells three functions its own way.
    class HookDialect final : public flux::sql::Dialect {
    public:
        HookDialect() noexcept : Dialect(flux::sql::dialect_caps_t {}) {}

        std::string_view name() const noexcept override { return "hook"; }
        std::string placeholder(size_t index) const override { return "?" + std::to_string(index); }
        bool is_reserved(std::string_view) const noexcept override { return false; }

        bool supports(const flux::ext::Slot* slot) const noexcept override {
            return slot->name != flux::slots::returning.name;
        }

        ers::Status before_slot(
            flux::ext::Context& ctx, const flux::sql::Query& query, const flux::ext::SlotBinding& binding
        ) const override {
            if (binding.slot->name == flux::slots::where.name && query.has(binding.slot))
                ctx.query += "\n/* before where */";

            return ers::ok;
        }

        ers::Result<flux::sql::function_spelling_t> function_spelling(std::string_view name, size_t arity) const override {
            if (name == "now")
                return flux::sql::function_spelling_t { .name = "CURRENT_TIMESTAMP", .call = false };

            if (name == "greatest")
                return flux::sql::function_spelling_t { .name = "max" };

            if (name == "forbidden")
                return ers::make_error(flux::ESqlError::Unsupported, "'{}' with {} arguments", name, arity);

            return flux::sql::function_spelling_t {};
        }
    };


    const HookDialect& hook() {
        static const HookDialect dialect;
        return dialect;
    }

    ers::Result<flux::sql::Statement> statement_of(const flux::sql::Query& query) {
        return query.to_statement(hook());
    }

    std::string sql_of(const flux::sql::Query& query) {
        const auto r = statement_of(query);

        return r ? (*r).sql : std::string("<error>");
    }

    std::string sql_of(const flux::sql::Expr& expr, const flux::sql::Dialect& dialect = hook()) {
        flux::sql::Context ctx(dialect);

        return expr.render(ctx) ? ctx.query : "<error>";
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
    CHECK(r.error().code() == flux::ESqlError::Unsupported);
}


// Slots -----------------------------------------------------------------------

FLUXMT_TEST_CASE("dialect refuses a slot it does not support", "[sql]") {
    const auto refused = statement_of(insert_into("t") | columns("a") | values(1) | returning("a"));

    REQUIRE(!refused);
    CHECK(refused.error().code() == flux::ESqlError::Unsupported);
    CHECK(refused.error().message().contains("hook"));

    CHECK(sql_of(insert_into("t") | columns("a") | values(1)) == "INSERT INTO t (a)\nVALUES (?1)");
}

FLUXMT_TEST_CASE("dialect writes text before a slot", "[sql]") {
    CHECK(sql_of(select_from("t") | where(col("a") == 1))
        == "SELECT *\nFROM t\n/* before where */\nWHERE a = ?1");

    CHECK(sql_of(select_from("t")) == "SELECT *\nFROM t");
}

FLUXMT_TEST_CASE("dialect hooks reach subqueries", "[sql]") {
    CHECK(sql_of(select_from("u") | where(col("id").in(select_from("o") | columns("user_id") | where(col("x") == 1))))
        == "SELECT *\nFROM u"
           "\n/* before where */"
           "\nWHERE id IN (SELECT user_id\nFROM o\n/* before where */\nWHERE x = ?1)");
}
