// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/sql.hpp>

// test
#include "support/sql/dialect.hpp"
#include "support/test_case.hpp"


using namespace flux::orm;

using std::string_literals::operator ""s;


// Details ---------------------------------------------------------------------

namespace {
    struct rendered_t {
        std::string sql;
        std::vector<flux::Value> params;
    };

    rendered_t render(const flux::sql::Expr& expr) {
        static const test::AntiDialect dialect;

        flux::sql::Context ctx(dialect);

        if (!expr.render(ctx))
            return { "<error>", {} };

        auto statement = std::move(ctx).finish();

        return { std::move(statement.sql), std::move(statement.params) };
    }

    std::string sql_of(const flux::sql::Expr& expr) {
        return render(expr).sql;
    }

    constexpr flux::ext::fn_t<"lower"> lower {};
    constexpr flux::ext::fn_t<"date_trunc"> date_trunc {};
    constexpr flux::ext::binary_op_t<"%"> similar {};

    struct NameColumn : flux::sql::ExprMethods {
        flux::sql::Expr to_expr() const { return col("name"); }
    };
}


// Rendering -------------------------------------------------------------------

FLUXMT_TEST_CASE("expr comparisons", "[sql]") {
    CHECK(sql_of(col("a") == 1) == "a = ?1");
    CHECK(sql_of(col("a") != "x") == "a <> ?1");
    CHECK(sql_of(5 <= col("a")) == "?1 <= a");
    CHECK(sql_of(col("a") == col("b")) == "a = b");
    CHECK(sql_of(col("t", "a") > 1) == "t.a > ?1");
}

FLUXMT_TEST_CASE("expr binds values in order", "[sql]") {
    const auto r = render(col("a") == 1 && col("b") == "x"s);

    CHECK(r.sql == "a = ?1 AND b = ?2");
    CHECK(r.params == std::vector<flux::Value>{ 1, std::string("x") });
}

FLUXMT_TEST_CASE("expr and or", "[sql]") {
    CHECK(sql_of(col("a") == 1 && col("b") == 2 && col("c") == 3) == "a = ?1 AND b = ?2 AND c = ?3");
    CHECK(sql_of(col("a") == 1 || (col("b") == 2 && col("c") == 3)) == "a = ?1 OR b = ?2 AND c = ?3");
    CHECK(sql_of((col("a") == 1 || col("b") == 2) && col("c") == 3) == "(a = ?1 OR b = ?2) AND c = ?3");
}

FLUXMT_TEST_CASE("expr arithmetic", "[sql]") {
    CHECK(sql_of(col("a") - (col("b") - col("c")) > 0) == "a - (b - c) > ?1");
    CHECK(sql_of(col("a") - col("b") - col("c") > 0) == "a - b - c > ?1");
    CHECK(sql_of((col("a") + col("b")) * col("c") > 0) == "(a + b) * c > ?1");
    CHECK(sql_of(col("a") + col("b") * col("c") > 0) == "a + b * c > ?1");
}

FLUXMT_TEST_CASE("expr not and null", "[sql]") {
    CHECK(sql_of(!(col("a") == 1)) == "NOT a = ?1");
    CHECK(sql_of(!(col("a") == 1 && col("b") == 2)) == "NOT (a = ?1 AND b = ?2)");
    CHECK(sql_of(col("a").is_null()) == "a IS NULL");
    CHECK(sql_of(col("a").is_not_null()) == "a IS NOT NULL");
    CHECK(sql_of(col("a").is_distinct_from(col("b"))) == "a IS DISTINCT FROM b");
    CHECK(sql_of((col("a") == 1).is_null()) == "a = ?1 IS NULL");
}

FLUXMT_TEST_CASE("expr in and between", "[sql]") {
    CHECK(sql_of(col("a").in({ 1, 2, 3 })) == "a IN (?1, ?2, ?3)");
    CHECK(sql_of(col("a").in(std::vector<std::string>{ "x", "y" })) == "a IN (?1, ?2)");
    CHECK(sql_of(col("a").in(std::vector<int>{})) == "FALSE");
    CHECK(sql_of(!col("a").in(std::vector<int>{})) == "NOT FALSE");
    CHECK(sql_of(col("a").between(1, 5)) == "a BETWEEN ?1 AND ?2");
    CHECK(sql_of(col("a").like("x%")) == "a LIKE ?1");
    CHECK(sql_of(col("a").contains(col("b"))) == "a @> b");
}

FLUXMT_TEST_CASE("expr follows the dialect capabilities", "[sql]") {
    CHECK(sql_of(col("a").ilike("x%")) == "lower(a) LIKE lower(?1)");

    CHECK(sql_of(col("a").any(std::vector<int>{ 1, 2 })) == "a IN (?1, ?2)");
    CHECK(sql_of(col("a").any(std::vector<int>{})) == "FALSE");
    CHECK(sql_of(col("a").any(col("b"))) == "<error>");

    CHECK(sql_of(col("a").cast("uuid") == col("b")) == "CAST(a AS uuid) = b");
    CHECK(sql_of((col("a") + col("b")).cast("int") > 1) == "CAST(a + b AS int) > ?1");
}

FLUXMT_TEST_CASE("expr row and null", "[sql]") {
    CHECK(sql_of(row(col("a"), col("b")) < row(1, 2)) == "(a, b) < (?1, ?2)");
    CHECK(sql_of(col("a") == null()) == "a = NULL");
    CHECK(sql_of(sql_bool(true)) == "TRUE");
    CHECK(sql_of(col("a").cast("int; --") == 1) == "<error>");
}

FLUXMT_TEST_CASE("expr identifiers are quoted, not trusted", "[sql]") {
    CHECK(sql_of(col("item") == 1) == "\"item\" = ?1");
    CHECK(sql_of(col("Item") == 1) == "\"Item\" = ?1");
    CHECK(sql_of(col("a; DROP TABLE t") == 1) == "\"a; DROP TABLE t\" = ?1");
    CHECK(sql_of(col("") == 1) == "<error>");
    CHECK(sql_of(col("a") == "'; DROP TABLE t; --"s) == "a = ?1");
}


// Extension -------------------------------------------------------------------

FLUXMT_TEST_CASE("expr user operators and functions", "[sql]") {
    CHECK(sql_of(similar(col("name"), "bob")) == "name % ?1");
    CHECK(sql_of(lower(col("email")) == "a@b.c") == "lower(email) = ?1");
    CHECK(sql_of(date_trunc("day", col("at")) == col("d")) == "date_trunc(?1, at) = d");
    CHECK(sql_of(similar(col("a"), col("b")) && col("c") == 1) == "a % b AND c = ?1");
}

FLUXMT_TEST_CASE("expr raw sql", "[sql]") {
    CHECK(sql_of(sql("{} @@ plainto_tsquery('russian', {})", col("search"), "q"))
        == "search @@ plainto_tsquery('russian', ?1)");

    CHECK(sql_of(sql("a OR b") && col("c") == 1) == "(a OR b) AND c = ?1");
    CHECK(sql_of(sql("{}")) == "<error>");
    CHECK(sql_of(sql("x", 1)) == "<error>");
}

FLUXMT_TEST_CASE("expr custom node", "[sql]") {
    struct ZeroLess final : flux::ext::IExprNode {
        flux::sql::Expr value;

        explicit ZeroLess(flux::sql::Expr v) : value(std::move(v)) {}

        ers::Status render(flux::ext::Context& ctx) const override {
            ctx.query += "0 < ";
            return value.render(ctx, flux::ext::EPrecedence::Comparison);
        }

        flux::ext::EPrecedence precedence() const noexcept override { return flux::ext::EPrecedence::Comparison; }
    };

    const auto custom = flux::ext::make_expr<ZeroLess>(col("x") + col("y"));

    CHECK(sql_of(custom && col("z") == 1) == "0 < x + y AND z = ?1");
}

FLUXMT_TEST_CASE("expr source with to_expr", "[sql]") {
    CHECK(sql_of(NameColumn{}.like("x%")) == "name LIKE ?1");
    CHECK(sql_of(NameColumn{}.is_null()) == "name IS NULL");
    CHECK(sql_of(NameColumn{} == 5 && col("a") == 1) == "name = ?1 AND a = ?2");
}


// Parameters ------------------------------------------------------------------

FLUXMT_TEST_CASE("expr param is bound once", "[sql]") {
    const auto at = param(5);

    const auto r = render(col("a") > at && col("b") < at && col("c") == 5);

    CHECK(r.sql == "a > ?1 AND b < ?1 AND c = ?2");
    CHECK(r.params == std::vector<flux::Value>{ 5, 5 });
}


// Aggregates ------------------------------------------------------------------

FLUXMT_TEST_CASE("expr aggregates", "[sql]") {
    CHECK(sql_of(count_all()) == "count(*)");
    CHECK(sql_of(count_distinct("a")) == "count(DISTINCT a)");
    CHECK(sql_of(corr_of("y", "x")) == "corr(y, x)");
    CHECK(sql_of(agg<"regr_slope">("y", col("x") * 2)) == "regr_slope(y, x * ?1)");
    CHECK(sql_of(coalesce(max_of("total"), 0)) == "coalesce(max(total), ?1)");
    CHECK(sql_of(string_agg("name", ", ")) == "string_agg(name, ?1)");
    CHECK(sql_of(percentile_cont(0.5, { "price" })) == "percentile_cont(?1) WITHIN GROUP (ORDER BY price)");
    CHECK(sql_of(mode_of({ "a" })) == "mode() WITHIN GROUP (ORDER BY a)");
    CHECK(sql_of(json_rows("items", { "position" })) == "json_agg(row_to_json(items) ORDER BY position)");
    CHECK(sql_of(mode_of({})) == "<error>");
}
