// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string>
#include <vector>

// sqlite
#include <sqlite3.h>

// fluxmt
#include <fluxmt/drivers/sqlite.hpp>

// test
#include "support/test_case.hpp"


namespace {
    using flux::impl::sqlite::connection;
    using flux::impl::sqlite::error;
    using flux::impl::sqlite::options_t;

    ers::i64 first(const flux::impl::sqlite::result_t& result) {
        return *result.rows.at(0).at(0).get_if<ers::i64>();
    }
}


FLUXMT_TEST_CASE("sqlite exec returns columns and rows", "[sqlite]") {
    connection conn(options_t{});

    const auto result = conn.exec("SELECT 1 AS one, 'a' AS two");

    REQUIRE(result.columns == std::vector<std::string> { "one", "two" });
    REQUIRE(result.rows.size() == 1);
    REQUIRE(first(result) == 1);
    REQUIRE(*result.rows[0][1].get_if<std::string>() == "a");
}

FLUXMT_TEST_CASE("sqlite exec reports affected rows and rowid", "[sqlite]") {
    connection conn(options_t{});
    conn.exec_script("CREATE TABLE t (id INTEGER PRIMARY KEY, name TEXT)");

    const auto inserted = conn.exec("INSERT INTO t (name) VALUES (?1)", { "a" });
    REQUIRE(inserted.affected == 1);
    REQUIRE(inserted.last_insert_rowid == 1);

    REQUIRE(conn.exec("UPDATE t SET name = 'b' WHERE id = 99").affected == 0);
    REQUIRE(conn.exec("UPDATE t SET name = 'b'").affected == 1);
}

FLUXMT_TEST_CASE("sqlite exec takes exactly one statement", "[sqlite]") {
    connection conn(options_t{});

    REQUIRE_NOTHROW(conn.exec("SELECT 1;"));
    REQUIRE_THROWS_AS(conn.exec("SELECT 1; SELECT 2"), error);
    REQUIRE_NOTHROW(conn.exec_script("CREATE TABLE a (x); CREATE TABLE b (x)"));
}

FLUXMT_TEST_CASE("sqlite parameter count must match", "[sqlite]") {
    connection conn(options_t{});

    try {
        conn.exec("SELECT ?1");
        FAIL("expected an error");
    }
    catch (const error& e) {
        REQUIRE(e.code() == SQLITE_RANGE);
    }
}

FLUXMT_TEST_CASE("sqlite enables foreign keys by default", "[sqlite]") {
    connection conn(options_t{});

    REQUIRE(first(conn.exec("PRAGMA foreign_keys")) == 1);
}

FLUXMT_TEST_CASE("sqlite double quotes are identifiers, not strings", "[sqlite]") {
    connection conn(options_t{});
    conn.exec_script("CREATE TABLE t (a)");

    REQUIRE_NOTHROW(conn.exec("SELECT \"a\" FROM t"));
    REQUIRE_THROWS_AS(conn.exec("SELECT \"nope\" FROM t"), error);
}

FLUXMT_TEST_CASE("sqlite open failure is a connection error", "[sqlite]") {
    options_t options;
    options.path = "/nonexistent/dir/fluxmt.db";

    try {
        connection conn(options);
        FAIL("expected an error");
    }
    catch (const error& e) {
        REQUIRE(flux::impl::sqlite::classify(e.code()) == flux::EDbError::ConnectionLost);
    }
}
