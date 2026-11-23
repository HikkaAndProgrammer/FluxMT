// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <cstdlib>
#include <string>

// fluxmt
#include <fluxmt/psql.hpp>

// test
#include "support/test_case.hpp"


namespace {
    flux::psql::options_t options_from_env() {
        auto get = [](const char* name, std::string fallback) {
            const char* value = std::getenv(name);
            return value ? std::string(value) : std::move(fallback);
            };

        return {
            .host = get("FLUXMT_TEST_PG_HOST", ""),
            .port = static_cast<ers::u16>(std::stoi(get("FLUXMT_TEST_PG_PORT", "5432"))),
            .dbname = get("FLUXMT_TEST_PG_DB", ""),
            .username = get("FLUXMT_TEST_PG_USER", ""),
            .password = get("FLUXMT_TEST_PG_PASSWORD", ""),
        };
    }
}


FLUXMT_TEST_CASE("live: commit, unique violation and bad sql", "[psql][live]") {
    if (!std::getenv("FLUXMT_TEST_PG_DB"))
        SKIP("FLUXMT_TEST_PG_DB is not set");

    flux::psql::Database db(options_from_env());

    auto one = flux::psql::with_tx(db, [](pqxx::work& tx) {
        return tx.query_value<int>("SELECT 1");
        });
    REQUIRE(one == 1);

    auto duplicate = flux::psql::try_with_tx(db, [](pqxx::work& tx) {
        tx.exec("CREATE TEMP TABLE t (id int PRIMARY KEY)");
        tx.exec("INSERT INTO t VALUES (1)");
        tx.exec("INSERT INTO t VALUES (1)");
        }, "duplicate");
    REQUIRE(!duplicate);
    REQUIRE(duplicate.error().code() == flux::EDbError::UniqueViolation);

    auto missing = flux::psql::try_with_tx(db, [](pqxx::work& tx) {
        tx.exec("SELECT * FROM table_that_does_not_exist");
        });
    REQUIRE(!missing);
    REQUIRE(missing.error().code() == flux::EDbError::Other);
}
