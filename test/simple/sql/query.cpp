// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <cstddef>
#include <string>
#include <string_view>
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
    // "?N" placeholders, no ILIKE, no native arrays: see support/sql/dialect.hpp
    ers::Result<flux::sql::Statement> statement_of(const flux::sql::Query& query) {
        static const test::AntiDialect dialect;

        return query.to_statement(dialect);
    }

    std::string sql_of(const flux::sql::Query& query) {
        const auto r = statement_of(query);

        return r ? (*r).sql : std::string("<error>");
    }

    bool contains(std::string_view haystack, std::string_view needle) {
        return haystack.find(needle) != std::string_view::npos;
    }

    class DistinctOnClause final : public flux::ext::IClause {
    public:
        explicit DistinctOnClause(std::vector<flux::sql::Expr> on) :
            IClause(&flux::slots::distinct),
            _on(std::move(on)) {
        }

        ers::Status render(flux::ext::Context& ctx) const override {
            ctx.query += " DISTINCT ON (";

            for (size_t i = 0; i < _on.size(); ++i) {
                if (i > 0)
                    ctx.query += ", ";

                ERS_TRY(_on[i].render(ctx));
            }

            ctx.query += ')';

            return ers::ok;
        }

    private:
        std::vector<flux::sql::Expr> _on;
    };
}


// Select ----------------------------------------------------------------------

FLUXMT_TEST_CASE("query select", "[sql]") {
    CHECK(sql_of(select_from("users")
        | columns("id", "name")
        | where(col("active") == true)
        | order_by("created_at", EOrder::Desc)
        | with_limit(20)
        | with_offset(5))
        == "SELECT id, name"
           "\nFROM users"
           "\nWHERE active = ?1"
           "\nORDER BY created_at DESC"
           "\nLIMIT ?2"
           "\nOFFSET ?3");

    CHECK(sql_of(select_from("users")) == "SELECT *\nFROM users");
    CHECK(sql_of(select_from("t") | distinct() | columns("a")) == "SELECT DISTINCT a\nFROM t");
}

FLUXMT_TEST_CASE("query statement carries the parameters", "[sql]") {
    const auto r = statement_of(select_from("t") | where(col("a") == 5 && col("b") == "x"s) | with_limit(10));

    REQUIRE(r);
    CHECK((*r).sql == "SELECT *\nFROM t\nWHERE a = ?1 AND b = ?2\nLIMIT ?3");
    CHECK((*r).params == std::vector<flux::Value>{ 5, std::string("x"), 10 });
}

FLUXMT_TEST_CASE("query where, having and the clause order", "[sql]") {
    CHECK(contains(sql_of(select_from("t") | where(col("a").is_null())), "WHERE a IS NULL"));
    CHECK(contains(sql_of(select_from("t") | where(col("a") != 1)), "WHERE a <> ?1"));

    // where() sits between ANDs, so a top-level OR is wrapped
    CHECK(sql_of(select_from("t") | where(col("a") == 1 || col("b") == 2) | where(col("c") == 3))
        == "SELECT *\nFROM t\nWHERE (a = ?1 OR b = ?2) AND c = ?3");

    // the order of the pipe does not matter, the layout does
    CHECK(sql_of(select_from("t") | with_limit(1) | order_by("a") | where(col("b") == 2))
        == "SELECT *\nFROM t\nWHERE b = ?1\nORDER BY a ASC\nLIMIT ?2");
}

FLUXMT_TEST_CASE("query single slot keeps the last clause", "[sql]") {
    CHECK(sql_of(select_from("t") | with_limit(1) | with_limit(2)) == "SELECT *\nFROM t\nLIMIT ?1");
}

FLUXMT_TEST_CASE("query join", "[sql]") {
    CHECK(sql_of(select_from("orders", "o")
        | join("users", "u", col("u.id") == col("o.user_id"))
        | where(col("u.active") == true))
        == "SELECT *\nFROM orders AS o\nJOIN users AS u ON u.id = o.user_id\nWHERE u.active = ?1");

    CHECK(contains(sql_of(select_from("t") | join("x; --", col("a") == col("b"))), "JOIN \"x; --\" ON a = b"));
}

FLUXMT_TEST_CASE("query select list", "[sql]") {
    CHECK(sql_of(select_from("orders")
        | columns("user_id", count_all().as("n"), sum_of("total").as("sum"))
        | column(coalesce(max_of("total"), 0).as("top"))
        | group_by("user_id")
        | having(count_all() > 5)
        | order_by(sum_of("total"), EOrder::Desc))
        == "SELECT user_id, count(*) AS n, sum(total) AS sum, coalesce(max(total), ?1) AS top"
           "\nFROM orders"
           "\nGROUP BY user_id"
           "\nHAVING count(*) > ?2"
           "\nORDER BY sum(total) DESC");

    CHECK(sql_of(select_from("t") | columns(count_distinct("a"), corr_of("y", "x"), agg<"regr_slope">("y", col("x") * 2)))
        == "SELECT count(DISTINCT a), corr(y, x), regr_slope(y, x * ?1)\nFROM t");

    CHECK(sql_of(select_from("t") | order_by(col("a") + col("b")) | order_by("c", EOrder::Desc))
        == "SELECT *\nFROM t\nORDER BY a + b ASC, c DESC");

    CHECK(sql_of(select_from("t") | order_by_random()) == "SELECT *\nFROM t\nORDER BY random() ASC");
}

FLUXMT_TEST_CASE("query aliases are quoted when they need it", "[sql]") {
    CHECK(sql_of(select_from("t") | column(col("a").as("x; --"))) == "SELECT a AS \"x; --\"\nFROM t");
    CHECK(sql_of(select_from("t") | column(col("a").as("A"))) == "SELECT a AS \"A\"\nFROM t");
}


// Insert, update, delete ------------------------------------------------------

FLUXMT_TEST_CASE("query insert", "[sql]") {
    CHECK(sql_of(insert_into("images")
        | columns("url", "path")
        | values("a.png"s, "/a.png"s)
        | on_conflict("url")
        | do_update("path")
        | returning("id"))
        == "INSERT INTO images (url, path)"
           "\nVALUES (?1, ?2)"
           "\nON CONFLICT (url)"
           "\nDO UPDATE SET path = excluded.path"
           "\nRETURNING id");

    CHECK(sql_of(insert_into("t") | columns("a", "at") | values("x", now()) | returning("id", col("at").as("created")))
        == "INSERT INTO t (a, at)\nVALUES (?1, now())\nRETURNING id, at AS created");

    CHECK(sql_of(insert_into("t") | set("a", 1) | returning_all())
        == "INSERT INTO t (a)\nVALUES (?1)\nRETURNING *");
}

FLUXMT_TEST_CASE("query insert with set", "[sql]") {
    CHECK(sql_of(insert_into("images")
        | set("url", "a.png"s, "path", "/a.png"s)
        | on_conflict("url")
        | do_update("path")
        | returning("id"))
        == "INSERT INTO images (url, path)"
           "\nVALUES (?1, ?2)"
           "\nON CONFLICT (url)"
           "\nDO UPDATE SET path = excluded.path"
           "\nRETURNING id");

    CHECK(sql_of(insert_into("t") | assign("a", 1)) == "INSERT INTO t (a)\nVALUES (?1)");
    CHECK(sql_of(insert_into("t") | set("a", 1, "b", 2, "c", 3)) == "INSERT INTO t (a, b, c)\nVALUES (?1, ?2, ?3)");
    CHECK(sql_of(insert_into("t") | set("a", "x"s, "at", now()) | returning("id"))
        == "INSERT INTO t (a, at)\nVALUES (?1, now())\nRETURNING id");

    CHECK(sql_of(insert_into("t") | set("a", 1, "b", 2) | on_conflict_constraint("t_a_key") | do_nothing())
        == "INSERT INTO t (a, b)\nVALUES (?1, ?2)\nON CONFLICT ON CONSTRAINT t_a_key\nDO NOTHING");

    // the order of the pipe does not matter
    CHECK(sql_of(insert_into("t") | returning("id") | set("a", 1, "b", 2))
        == "INSERT INTO t (a, b)\nVALUES (?1, ?2)\nRETURNING id");
}

FLUXMT_TEST_CASE("query insert values are parameters", "[sql]") {
    const auto sql = sql_of(insert_into("t") | set("a", "'; DROP TABLE t; --"s, "b", 1));

    CHECK(contains(sql, "VALUES (?1, ?2)"));
    CHECK_FALSE(contains(sql, "DROP"));
}

FLUXMT_TEST_CASE("query insert mistakes", "[sql]") {
    CHECK_FALSE(statement_of(insert_into("t") | columns("a") | set("b", 1, "c", 2)));
    CHECK_FALSE(statement_of(insert_into("t") | set("a", 1, "b", 2) | values(3)));
    CHECK_FALSE(statement_of(insert_into("t") | columns("a") | values(1) | assign("b", 2)));

    const auto mixed = statement_of(insert_into("t") | columns("a") | values(1) | set("b", 2, "c", 3));

    REQUIRE_FALSE(mixed);
    CHECK(mixed.error().code() == flux::ESqlError::SlotConflict);

    CHECK_FALSE(statement_of(insert_into("t") | columns("a") | values()));
}

FLUXMT_TEST_CASE("query update and delete", "[sql]") {
    CHECK(sql_of(update("users") | assign("name", "bob"s) | where(col("id") == 1))
        == "UPDATE users\nSET name = ?1\nWHERE id = ?2");

    CHECK(sql_of(update("users") | set("name", "bob"s, "visits", col("visits") + 1, "seen_at", now()) | where(col("id") == 1))
        == "UPDATE users\nSET name = ?1, visits = visits + ?2, seen_at = now()\nWHERE id = ?3");

    CHECK(sql_of(update("products") | assign("stock", col("stock") - 3) | where(col("id") == 1))
        == "UPDATE products\nSET stock = stock - ?1\nWHERE id = ?2");

    CHECK(sql_of(update("t") | assign_null("a")) == "UPDATE t\nSET a = NULL");

    CHECK(sql_of(delete_from("users") | where(col("id") == 1)) == "DELETE FROM users\nWHERE id = ?1");
}


// Names and slots -------------------------------------------------------------

FLUXMT_TEST_CASE("query identifiers are quoted, not trusted", "[sql]") {
    CHECK(sql_of(select_from("users; --")) == "SELECT *\nFROM \"users; --\"");
    CHECK(sql_of(select_from("item")) == "SELECT *\nFROM \"item\"");
    CHECK(sql_of(select_from("users") | columns("1")) == "SELECT \"1\"\nFROM users");
    CHECK(sql_of(insert_into("t") | set("x; --", 1, "b", 2)) == "INSERT INTO t (\"x; --\", b)\nVALUES (?1, ?2)");

    CHECK(sql_of(select_from("public.users") | where(col("users.id") == 1))
        == "SELECT *\nFROM public.users\nWHERE users.id = ?1");

    CHECK_FALSE(statement_of(select_from("")));
    CHECK_FALSE(statement_of(select_from("a..b")));
}

FLUXMT_TEST_CASE("query rejects what the layout of the dialect does not have", "[sql]") {
    const auto unsupported = statement_of(delete_from("t") | order_by("a"));

    REQUIRE_FALSE(unsupported);
    CHECK(unsupported.error().code() == flux::ESqlError::Unsupported);
    CHECK(unsupported.error().message().contains("anti"));
}

FLUXMT_TEST_CASE("query with a layout of its own", "[sql]") {
    static constexpr flux::ext::SlotBinding own[] = {
        flux::impl::shared::select_keyword,
        flux::impl::shared::from,
    };

    auto q = flux::sql::Query(own);
    q |= from("t");

    CHECK(sql_of(q) == "SELECT\nFROM t");

    const auto foreign = statement_of(q | order_by("a"));

    REQUIRE_FALSE(foreign);
    CHECK(foreign.error().code() == flux::ESqlError::ForeignSlot);
}

FLUXMT_TEST_CASE("query bounds are not negative", "[sql]") {
    const auto negative = statement_of(select_from("t") | with_limit(-1));

    REQUIRE_FALSE(negative);
    CHECK(negative.error().code() == flux::ESqlError::NegativeBound);
}

FLUXMT_TEST_CASE("query raw clause binds its values", "[sql]") {
    const auto r = statement_of(select_from("t") | raw(&flux::slots::where, "a = ? AND b = ?", 1, "x"));

    REQUIRE(r);
    CHECK((*r).sql == "SELECT *\nFROM t\nWHERE a = ?1 AND b = ?2");
    CHECK((*r).params == std::vector<flux::Value>{ 1, std::string("x") });

    const auto few = statement_of(select_from("t") | raw(&flux::slots::where, "a = ? AND b = ?", 1));

    REQUIRE_FALSE(few);
    CHECK(few.error().code() == flux::ESqlError::PlaceholderMismatch);

    CHECK_FALSE(statement_of(select_from("t") | raw(&flux::slots::where, "a = ?", 1, 2)));
}

FLUXMT_TEST_CASE("query layout can be extended", "[sql]") {
    static constexpr flux::ext::Slot extra { .name = "test.extra", .arity = flux::ext::EArity::Single };

    const test::AntiDialect dialect;

    auto q = select_from("t");

    REQUIRE(q.insert_after(&flux::slots::from, { .slot = &extra, .prefix = "\n-- " }));
    q |= raw(&extra, "extra");

    const auto layout = q.layout(dialect);

    REQUIRE(layout);
    CHECK((*layout).contains(&extra));
    CHECK(*(*layout).index_of(&extra) == *(*layout).index_of(&flux::slots::from) + 1);

    CHECK(sql_of(q) == "SELECT *\nFROM t\n-- extra");

    CHECK_FALSE(select_from("t").insert_after(&flux::slots::from, { .slot = nullptr }));
    CHECK_FALSE(select_from("t").insert_after(nullptr, { .slot = &extra }));
}

FLUXMT_TEST_CASE("query layout changes are checked against the layout of the dialect", "[sql]") {
    static constexpr flux::ext::Slot extra { .name = "test.extra", .arity = flux::ext::EArity::Single };

    auto twice = select_from("t");
    REQUIRE(twice.insert_after(&flux::slots::from, { .slot = &extra }));
    REQUIRE(twice.insert_after(&flux::slots::from, { .slot = &extra }));

    const auto duplicate = statement_of(twice);

    REQUIRE_FALSE(duplicate);
    CHECK(duplicate.error().code() == flux::ESqlError::DuplicateSlot);

    auto lost = select_from("t");
    REQUIRE(lost.insert_after(&flux::slots::returning, { .slot = &extra }));

    const auto unknown = statement_of(lost);

    REQUIRE_FALSE(unknown);
    CHECK(unknown.error().code() == flux::ESqlError::UnknownAnchor);
}

FLUXMT_TEST_CASE("query custom clause", "[sql]") {
    CHECK(sql_of(select_from("prices")
        | flux::ext::make_clause<DistinctOnClause>(std::vector{ col("product_id") })
        | order_by("product_id")
        | order_by("at", EOrder::Desc))
        == "SELECT DISTINCT ON (product_id) *\nFROM prices\nORDER BY product_id ASC, at DESC");
}

FLUXMT_TEST_CASE("query when", "[sql]") {
    const auto off = false;
    const auto on = true;

    CHECK(sql_of(select_from("t") | when(off, where(col("a") == 1)) | when(on, where(col("b") == 2)))
        == "SELECT *\nFROM t\nWHERE b = ?1");

    CHECK(sql_of(select_from("t") | when(on, [] { return where(col("a") == 1); }))
        == "SELECT *\nFROM t\nWHERE a = ?1");

    bool built = false;
    auto skipped = select_from("t") | when(off, [&] {
        built = true;
        return where(col("a") == 1);
    });

    CHECK_FALSE(built);
    CHECK(sql_of(skipped) == "SELECT *\nFROM t");
}


// Subqueries ------------------------------------------------------------------

FLUXMT_TEST_CASE("query subqueries", "[sql]") {
    const auto r = statement_of(select_from("users")
        | columns("id")
        | where(col("active") == true)
        | where(exists(select_from("orders") | where(col("user_id") == 7)))
        | with_limit(10));

    REQUIRE(r);
    CHECK(contains((*r).sql, "WHERE active = ?1 AND EXISTS (SELECT *"));
    CHECK(contains((*r).sql, "WHERE user_id = ?2"));
    CHECK(contains((*r).sql, "LIMIT ?3"));

    CHECK(sql_of(select_from("users")
        | where(col("id").in(select_from("orders") | columns("user_id") | where(col("total") > 100))))
        == "SELECT *\nFROM users\nWHERE id IN (SELECT user_id\nFROM orders\nWHERE total > ?1)");
}

FLUXMT_TEST_CASE("query select subquery", "[sql]") {
    const auto orders = select_from("orders") | column(count_all()) | where(col("user_id") == col("u.id"));

    CHECK(sql_of(select_from("users", "u") | columns("u.id", subquery(orders).as("orders")))
        == "SELECT u.id, (SELECT count(*)\nFROM orders\nWHERE user_id = u.id) AS orders\nFROM users AS u");
}

FLUXMT_TEST_CASE("query exists_in", "[sql]") {
    CHECK(sql_of(exists_in("orders") | where(col("user_id") == 7))
        == "SELECT EXISTS (\nSELECT 1 FROM orders\nWHERE user_id = ?1\n) AS \"exists\"");
}

FLUXMT_TEST_CASE("query cte and union", "[sql]") {
    CHECK(sql_of(select_from("recent") | with_cte("recent", select_from("t") | where(col("a") > 1)))
        == "WITH recent AS (\nSELECT *\nFROM t\nWHERE a > ?1\n)\nSELECT *\nFROM recent");

    CHECK(sql_of(select_from("a") | columns("x") | union_with(select_from("b") | columns("x"), true))
        == "SELECT x\nFROM a\nUNION ALL\nSELECT x\nFROM b");

    CHECK(sql_of(select_from("a") | columns("x") | union_with(select_from("b") | columns("x")))
        == "SELECT x\nFROM a\nUNION\nSELECT x\nFROM b");
}
