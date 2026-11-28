// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <chrono>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/drivers/sqlite.hpp>

// test
#include "support/drivers/sqlite/db.hpp"
#include "support/test_case.hpp"


using namespace flux::sqlite::orm;

using std::string_literals::operator ""s;


// Details ---------------------------------------------------------------------

namespace {
    using namespace std::chrono_literals;
    namespace sq = flux::sqlite;

    sq::Database open_db(const test::TempDb& file) {
        return sq::Database(file.options(), {
            .min_size        = 0,
            .max_size        = 2,
            .acquire_timeout = 500ms
        });
    }

    void create_images(sq::Database& db) {
        flux::with_tx(db, [](sq::transaction& tx) {
            tx.exec(R"(
                CREATE TABLE images (
                    id     INTEGER PRIMARY KEY,
                    url    TEXT    NOT NULL UNIQUE,
                    path   TEXT    NOT NULL,
                    active INTEGER NOT NULL DEFAULT 1
                )
            )");
        }, "schema");
    }

    void seed(sq::Database& db) {
        flux::with_query(db, flux::statement_t {
            "INSERT INTO images (url, path) VALUES ('a', '/a'), ('b', '/b'), ('c', '/c')", {}
        });
    }

    size_t images(sq::Database& db) {
        return flux::with_query(db, select_from("images") | order_by("id")).rows.size();
    }

    ers::i64 integer(const sq::result_t& result, size_t row = 0) {
        return *result.rows.at(row).at(0).get_if<ers::i64>();
    }
}


// Statements ------------------------------------------------------------------

FLUXMT_TEST_CASE("sqlite with_query runs a builder through the pool", "[sqlite]") {
    test::TempDb file;
    auto db = open_db(file);
    create_images(db);

    const auto inserted = flux::with_query(db, insert_into("images")
        | columns("url", "path")
        | values("a"s, "/a"s)
        | returning("id"));

    REQUIRE(inserted.rows.size() == 1);
    CHECK(integer(inserted) == 1);

    const auto found = flux::with_query(db, select_from("images") | where(col("url") == "a"s));

    REQUIRE(found.rows.size() == 1);
    CHECK(images(db) == 1);
}

FLUXMT_TEST_CASE("sqlite with_query runs a raw statement as it is", "[sqlite]") {
    test::TempDb file;
    auto db = open_db(file);

    const auto sum = flux::with_query(db, flux::statement_t { "SELECT ?1 + ?2", { 2, 3 } });

    REQUIRE(sum.rows.size() == 1);
    CHECK(integer(sum) == 5);
}

FLUXMT_TEST_CASE("sqlite with_query reports the affected rows", "[sqlite]") {
    test::TempDb file;
    auto db = open_db(file);
    create_images(db);
    seed(db);

    const auto updated = flux::with_query(db, update("images")
        | assign("path", "/x"s)
        | where(col("id") <= 2));

    CHECK(updated.affected == 2);

    const auto removed = flux::with_query(db, delete_from("images") | where(col("id") == 1));

    CHECK(removed.affected == 1);
    CHECK(images(db) == 2);
}

FLUXMT_TEST_CASE("sqlite with_query passes every kind of parameter", "[sqlite]") {
    test::TempDb file;
    auto db = open_db(file);

    const flux::value_bytes_t blob { std::byte { 1 }, std::byte { 0 }, std::byte { 255 } };

    const auto echoed = flux::with_query(db, flux::statement_t {
        "SELECT ?1, ?2, ?3, ?4, ?5",
        { ers::i64 { -7 }, 1.5, std::string("hello"), flux::Value {}, blob }
    });

    REQUIRE(echoed.rows.size() == 1);
    CHECK(echoed.rows[0] == std::vector<flux::Value> { ers::i64 { -7 }, 1.5, std::string("hello"), flux::Value {}, blob });
}


// Transactions ----------------------------------------------------------------

FLUXMT_TEST_CASE("sqlite with_query takes any transaction of the callable", "[sqlite]") {
    test::TempDb file;
    auto db = open_db(file);
    create_images(db);

    flux::with_tx(db, [](sq::transaction& tx) {
        flux::with_query(tx, insert_into("images") | columns("url", "path") | values("a"s, "/a"s));
    });

    flux::with_tx(db, [](sq::Tx& tx) {
        flux::with_query(tx, insert_into("images") | columns("url", "path") | values("b"s, "/b"s));
    });

    flux::with_tx(db, [](sq::write_transaction& tx) {
        flux::with_query(tx, insert_into("images") | columns("url", "path") | values("c"s, "/c"s));
    });

    CHECK(images(db) == 3);
}

FLUXMT_TEST_CASE("sqlite with_query on a lease", "[sqlite]") {
    test::TempDb file;
    auto db = open_db(file);
    create_images(db);
    seed(db);

    auto lease = db.acquire_or_throw();

    const auto rows = flux::with_tx(lease, [](sq::transaction& tx) {
        return flux::with_query(tx, select_from("images") | order_by("id"));
    });

    CHECK(rows.rows.size() == 3);
}

FLUXMT_TEST_CASE("sqlite queries of one transaction commit and roll back together", "[sqlite]") {
    test::TempDb file;
    auto db = open_db(file);
    create_images(db);

    flux::with_tx(db, [](sq::transaction& tx) {
        flux::with_query(tx, insert_into("images") | columns("url", "path") | values("a"s, "/a"s));
        flux::with_query(tx, insert_into("images") | columns("url", "path") | values("b"s, "/b"s));
    }, "two");

    REQUIRE(images(db) == 2);

    REQUIRE_THROWS_AS(flux::with_tx(db, [](sq::transaction& tx) {
        flux::with_query(tx, insert_into("images") | columns("url", "path") | values("c"s, "/c"s));
        throw std::runtime_error("changed my mind");
    }, "three"), std::runtime_error);

    CHECK(images(db) == 2);
}


// Errors ----------------------------------------------------------------------

FLUXMT_TEST_CASE("sqlite with_query throws when a statement cannot be built", "[sqlite]") {
    test::TempDb file;
    auto db = open_db(file);
    create_images(db);

    REQUIRE_THROWS(flux::with_query(db, insert_into("images").erased()
        | columns("url")
        | values("a"s)
        | on_conflict_constraint("images_url_key")
        | do_nothing()));
}

FLUXMT_TEST_CASE("sqlite try_with_query returns a Result", "[sqlite]") {
    test::TempDb file;
    auto db = open_db(file);
    create_images(db);
    seed(db);

    const auto fine = flux::try_with_query(db, select_from("images") | order_by("id"), "list");

    REQUIRE(fine);
    CHECK((*fine).rows.size() == 3);

    const auto refused = flux::try_with_query(db, insert_into("images").erased()
        | columns("url")
        | values("d"s)
        | on_conflict_constraint("images_url_key")
        | do_nothing());

    REQUIRE(!refused);
    CHECK(refused.error().code() == flux::EQueryError::ForeignSlot);

    const auto duplicate = flux::try_with_query(db, insert_into("images")
        | columns("url", "path")
        | values("a"s, "/z"s));

    REQUIRE(!duplicate);
    CHECK(duplicate.error().code() == flux::EDbError::UniqueViolation);

    CHECK(images(db) == 3);
}

FLUXMT_TEST_CASE("sqlite a failed try_with_query inside a transaction rolls it back", "[sqlite]") {
    test::TempDb file;
    auto db = open_db(file);
    create_images(db);

    const auto outcome = flux::try_with_tx(db, [](sq::transaction& tx) -> ers::Result<int> {
        flux::with_query(tx, insert_into("images") | columns("url", "path") | values("a"s, "/a"s));

        auto failed = flux::try_with_query(tx, insert_into("images") | columns("url", "path") | values("a"s, "/z"s));
        if (!failed)
            return std::move(failed).extract_error();

        return 1;
    }, "rollback");

    REQUIRE(!outcome);
    CHECK(outcome.error().code() == flux::EDbError::UniqueViolation);
    CHECK(images(db) == 0);
}
