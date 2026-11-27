// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/drivers/sqlite.hpp>
#include <fluxmt/sql.hpp>

// test
#include "support/test_case.hpp"


using namespace flux::sqlite::orm;

using std::string_literals::operator ""s;


// Details ---------------------------------------------------------------------

namespace {
    flux::sqlite::result_t run(flux::sqlite::connection& conn, const flux::sql::Query& query) {
        auto statement = query.to_statement(flux::sqlite::dialect());
        REQUIRE(statement);

        return conn.exec((*statement).text, (*statement).params);
    }

    ers::i64 integer(const flux::sqlite::result_t& result, size_t row = 0) {
        return *result.rows.at(row).at(0).get_if<ers::i64>();
    }

    void seed(flux::sqlite::connection& conn) {
        conn.exec_script(R"(
            CREATE TABLE images (
                id     INTEGER PRIMARY KEY,
                url    TEXT    NOT NULL UNIQUE,
                path   TEXT    NOT NULL,
                active INTEGER NOT NULL DEFAULT 1
            );

            INSERT INTO images (url, path, active) VALUES ('a', '/a', 1), ('b', '/b', 1), ('c', '/c', 0);
        )");
    }
}


// Generated statements --------------------------------------------------------

FLUXMT_TEST_CASE("sqlite runs a generated select", "[sqlite]") {
    flux::sqlite::connection conn(flux::sqlite::options_t{});
    seed(conn);

    CHECK(run(conn, select_from("images") | where(col("active") == true)).rows.size() == 2);
    CHECK(run(conn, select_from("images") | where(col("active") == true) | with_limit(1)).rows.size() == 1);
}

FLUXMT_TEST_CASE("sqlite runs a generated insert with returning", "[sqlite]") {
    flux::sqlite::connection conn(flux::sqlite::options_t{});
    seed(conn);

    const auto inserted = run(conn, insert_into("images")
        | columns("url", "path")
        | values("d"s, "/d"s)
        | returning("id"));

    REQUIRE(inserted.rows.size() == 1);
    CHECK(integer(inserted) == 4);
}

FLUXMT_TEST_CASE("sqlite runs a generated upsert", "[sqlite]") {
    flux::sqlite::connection conn(flux::sqlite::options_t{});
    seed(conn);

    auto upsert = [&](std::string path) {
        return run(conn, insert_into("images")
            | columns("url", "path")
            | values("a"s, std::move(path))
            | on_conflict("url")
            | do_update("path")
            | returning("id"));
    };

    CHECK(integer(upsert("/first")) == 1);
    CHECK(integer(upsert("/second")) == 1);

    CHECK(integer(conn.exec("SELECT count(*) FROM images")) == 3);
    CHECK(*conn.exec("SELECT path FROM images WHERE id = 1").rows.at(0).at(0).get_if<std::string>() == "/second");
}

FLUXMT_TEST_CASE("sqlite runs a spread array", "[sqlite]") {
    flux::sqlite::connection conn(flux::sqlite::options_t{});
    seed(conn);

    CHECK(run(conn, select_from("images") | where(col("id").any(std::vector<int>{ 1, 3 }))).rows.size() == 2);
    CHECK(run(conn, select_from("images") | where(col("id").any(std::vector<int>{}))).rows.empty());
}

FLUXMT_TEST_CASE("sqlite runs the fallbacks for missing features", "[sqlite]") {
    flux::sqlite::connection conn(flux::sqlite::options_t{});
    seed(conn);

    CHECK(run(conn, select_from("images") | where(col("path").ilike("/A%"))).rows.size() == 1);
    CHECK(run(conn, select_from("images") | where(col("id").cast("text") == "1")).rows.size() == 1);
}

FLUXMT_TEST_CASE("sqlite runs OFFSET without LIMIT", "[sqlite]") {
    flux::sqlite::connection conn(flux::sqlite::options_t{});
    seed(conn);

    const auto skipped = run(conn, select_from("images") | order_by("id") | with_offset(2));

    REQUIRE(skipped.rows.size() == 1);
    CHECK(integer(skipped) == 3);
}

FLUXMT_TEST_CASE("sqlite runs its own spelling of functions", "[sqlite]") {
    flux::sqlite::connection conn(flux::sqlite::options_t{});
    seed(conn);

    const auto stamp = run(conn, select_from("images") | column(now()) | with_limit(1));

    REQUIRE(stamp.rows.size() == 1);
    CHECK(stamp.rows[0][0].get_if<std::string>()->size() == 19);

    CHECK(integer(run(conn, select_from("images") | column(greatest(1, 5)) | with_limit(1))) == 5);
    CHECK(integer(run(conn, select_from("images") | column(least(7, 2, 9)) | with_limit(1))) == 2);
}

FLUXMT_TEST_CASE("sqlite runs quoted reserved words", "[sqlite]") {
    flux::sqlite::connection conn(flux::sqlite::options_t{});
    conn.exec_script(R"(
        CREATE TABLE "order" (id INTEGER, "limit" INTEGER);
        INSERT INTO "order" VALUES (1, 10), (2, 20);
    )");

    CHECK(run(conn, select_from("order") | where(col("limit") == 20)).rows.size() == 1);
}
