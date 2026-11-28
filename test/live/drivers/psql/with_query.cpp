// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/drivers/psql.hpp>

// test
#include "support/drivers/psql/live.hpp"
#include "support/test_case.hpp"


using namespace flux::psql::orm;

using std::string_literals::operator ""s;


// Details ---------------------------------------------------------------------

namespace {
    using test::live::Sandbox;

    std::string images_of(const Sandbox& sb) {
        return sb.schema() + ".images";
    }

    void create_images(Sandbox& sb) {
        flux::with_tx(sb.db(), [&](pqxx::work& tx) {
            tx.exec(sb.sql(R"(
                CREATE TABLE @.images (
                    id     int     GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                    url    text    NOT NULL UNIQUE,
                    path   text    NOT NULL,
                    active boolean NOT NULL DEFAULT true
                )
            )"));
        }, "schema");
    }

    void seed(Sandbox& sb) {
        flux::with_query(sb.db(), flux::statement_t {
            sb.sql("INSERT INTO @.images (url, path) VALUES ('a', '/a'), ('b', '/b'), ('c', '/c')"), {}
        });
    }

    size_t images(Sandbox& sb) {
        return flux::with_query(sb.db(), select_from(images_of(sb)) | order_by("id")).size();
    }
}


// Statements ------------------------------------------------------------------

FLUXMT_TEST_CASE("psql with_query runs a builder through the pool", "[psql]") {
    Sandbox sb;
    create_images(sb);

    const auto inserted = flux::with_query(sb.db(), insert_into(images_of(sb))
        | columns("url", "path")
        | values("a"s, "/a"s)
        | returning("id"));

    REQUIRE(inserted.size() == 1);
    CHECK(inserted[0][0].as<int>() == 1);

    const auto found = flux::with_query(sb.db(), select_from(images_of(sb)) | where(col("url") == "a"s));

    REQUIRE(found.size() == 1);
    CHECK(found[0]["path"].as<std::string>() == "/a");
}

FLUXMT_TEST_CASE("psql with_query runs a raw statement as it is", "[psql]") {
    Sandbox sb;

    const auto sum = flux::with_query(sb.db(), flux::statement_t { "SELECT $1::int + $2::int", { 2, 3 } });

    REQUIRE(sum.size() == 1);
    CHECK(sum[0][0].as<int>() == 5);
}

FLUXMT_TEST_CASE("psql with_query reports the affected rows", "[psql]") {
    Sandbox sb;
    create_images(sb);
    seed(sb);

    const auto updated = flux::with_query(sb.db(), update(images_of(sb))
        | assign("path", "/x"s)
        | where(col("id") <= 2));

    CHECK(updated.affected_rows() == 2);

    const auto removed = flux::with_query(sb.db(), delete_from(images_of(sb)) | where(col("id") == 1));

    CHECK(removed.affected_rows() == 1);
    CHECK(images(sb) == 2);
}

FLUXMT_TEST_CASE("psql with_query binds an array as one parameter", "[psql]") {
    Sandbox sb;
    create_images(sb);
    seed(sb);

    const auto both = flux::with_query(sb.db(), select_from(images_of(sb))
        | where(col("id").any(std::vector<int> { 1, 3 }))
        | order_by("id"));

    REQUIRE(both.size() == 2);
    CHECK(both[0]["url"].as<std::string>() == "a");
    CHECK(both[1]["url"].as<std::string>() == "c");
}

FLUXMT_TEST_CASE("psql with_query passes every kind of parameter", "[psql]") {
    Sandbox sb;

    const auto echoed = flux::with_query(sb.db(), flux::statement_t {
        "SELECT $1::bigint, $2::float8, $3::text, $4::text, $5::boolean",
        { ers::i64 { -7 }, 1.5, std::string("hello"), flux::Value {}, true }
    });

    REQUIRE(echoed.size() == 1);
    CHECK(echoed[0][0].as<ers::i64>() == -7);
    CHECK(echoed[0][1].as<double>() == 1.5);
    CHECK(echoed[0][2].as<std::string>() == "hello");
    CHECK(echoed[0][3].is_null());
    CHECK(echoed[0][4].as<bool>());
}


// Transactions ----------------------------------------------------------------

FLUXMT_TEST_CASE("psql with_query takes any transaction of the callable", "[psql]") {
    Sandbox sb;
    create_images(sb);

    flux::with_tx(sb.db(), [&](pqxx::work& tx) {
        flux::with_query(tx, insert_into(images_of(sb)) | columns("url", "path") | values("a"s, "/a"s));
    });

    flux::with_tx(sb.db(), [&](flux::psql::Tx& tx) {
        flux::with_query(tx, insert_into(images_of(sb)) | columns("url", "path") | values("b"s, "/b"s));
    });

    const auto rows = flux::with_tx(sb.db(), [&](pqxx::read_transaction& tx) {
        return flux::with_query(tx, select_from(images_of(sb)) | order_by("id"));
    });

    CHECK(rows.size() == 2);
}

FLUXMT_TEST_CASE("psql with_query on a lease", "[psql]") {
    Sandbox sb;
    create_images(sb);
    seed(sb);

    auto lease = sb.db().acquire_or_throw();

    const auto rows = flux::with_tx(lease, [&](pqxx::work& tx) {
        return flux::with_query(tx, select_from(images_of(sb)) | order_by("id"));
    });

    CHECK(rows.size() == 3);
}

FLUXMT_TEST_CASE("psql queries of one transaction commit and roll back together", "[psql]") {
    Sandbox sb;
    create_images(sb);

    flux::with_tx(sb.db(), [&](pqxx::work& tx) {
        flux::with_query(tx, insert_into(images_of(sb)) | columns("url", "path") | values("a"s, "/a"s));
        flux::with_query(tx, insert_into(images_of(sb)) | columns("url", "path") | values("b"s, "/b"s));
    }, "two");

    REQUIRE(images(sb) == 2);

    REQUIRE_THROWS_AS(flux::with_tx(sb.db(), [&](pqxx::work& tx) {
        flux::with_query(tx, insert_into(images_of(sb)) | columns("url", "path") | values("c"s, "/c"s));
        throw std::runtime_error("changed my mind");
    }, "three"), std::runtime_error);

    CHECK(images(sb) == 2);
}


// Errors ----------------------------------------------------------------------

FLUXMT_TEST_CASE("psql with_query throws when a statement cannot be built", "[psql]") {
    Sandbox sb;
    create_images(sb);

    REQUIRE_THROWS(flux::with_query(sb.db(), select_from(images_of(sb)).erased() | returning("id")));
}

FLUXMT_TEST_CASE("psql try_with_query returns a Result", "[psql]") {
    Sandbox sb;
    create_images(sb);
    seed(sb);

    const auto fine = flux::try_with_query(sb.db(), select_from(images_of(sb)) | order_by("id"), "list");

    REQUIRE(fine);
    CHECK((*fine).size() == 3);

    const auto refused = flux::try_with_query(sb.db(), select_from(images_of(sb)).erased() | returning("id"));

    REQUIRE(!refused);
    CHECK(refused.error().code() == flux::EQueryError::ForeignSlot);

    const auto duplicate = flux::try_with_query(sb.db(), insert_into(images_of(sb))
        | columns("url", "path")
        | values("a"s, "/z"s));

    REQUIRE(!duplicate);
    CHECK(duplicate.error().code() == flux::EDbError::UniqueViolation);

    CHECK(images(sb) == 3);
}

FLUXMT_TEST_CASE("psql a failed try_with_query inside a transaction rolls it back", "[psql]") {
    Sandbox sb;
    create_images(sb);

    const auto outcome = flux::try_with_tx(sb.db(), [&](pqxx::work& tx) -> ers::Result<int> {
        flux::with_query(tx, insert_into(images_of(sb)) | columns("url", "path") | values("a"s, "/a"s));

        auto failed = flux::try_with_query(tx, insert_into(images_of(sb))
            | columns("url", "path")
            | values("a"s, "/z"s));

        if (!failed)
            return std::move(failed).extract_error();

        return 1;
    }, "rollback");

    REQUIRE(!outcome);
    CHECK(outcome.error().code() == flux::EDbError::UniqueViolation);
    CHECK(images(sb) == 0);
}
