// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <vector>

// fluxmt
#include <fluxmt/psql.hpp>

// test
#include "support/psql/live.hpp"
#include "support/test_case.hpp"


namespace {
    using Sandbox = test::live::Sandbox;

    void create_tables(Sandbox& sb) {
        flux::psql::with_tx(sb.db(), [&](pqxx::work& tx) {
            tx.exec(sb.sql(R"(
                CREATE TABLE @.account (
                    id      int    GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                    email   text   NOT NULL UNIQUE,
                    balance bigint NOT NULL DEFAULT 0 CHECK (balance >= 0)
                )
            )"));
            tx.exec(sb.sql(R"(
                CREATE TABLE @.entry (
                    id         int    GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                    account_id int    NOT NULL REFERENCES @.account (id),
                    amount     bigint NOT NULL
                )
            )"));
            }, "create tables");
    }

    void seed(Sandbox& sb) {
        flux::psql::with_tx(sb.db(), [&](pqxx::work& tx) {
            const auto insert = sb.sql("INSERT INTO @.account (email, balance) VALUES ($1, $2)");
            tx.exec(insert, pqxx::params{ "alice.example", 100 });
            tx.exec(insert, pqxx::params{ "bob.example", 50 });
            }, "seed");
    }

    long long accounts(Sandbox& sb) {
        return sb.scalar<long long>("SELECT count(*) FROM @.account");
    }

    long long balance_of(Sandbox& sb, const char* email) {
        return sb.scalar<long long>(std::format("SELECT balance FROM @.account WHERE email = '{}'", email));
    }
}


FLUXMT_TEST_CASE("psql live: create, fill, read, update, delete", "[psql]") {
    Sandbox sb;
    create_tables(sb);
    seed(sb);

    REQUIRE(accounts(sb) == 2);
    REQUIRE(balance_of(sb, "alice.example") == 100);

    flux::psql::with_tx(sb.db(), [&](pqxx::work& tx) {
        tx.exec(sb.sql("UPDATE @.account SET balance = balance - 30 WHERE email = 'alice.example'"));
        tx.exec(sb.sql("UPDATE @.account SET balance = balance + 30 WHERE email = 'bob.example'"));
        }, "transfer");

    REQUIRE(balance_of(sb, "alice.example") == 70);
    REQUIRE(balance_of(sb, "bob.example") == 80);

    flux::psql::with_tx(sb.db(), [&](pqxx::work& tx) {
        tx.exec(sb.sql("DELETE FROM @.account WHERE email = 'bob.example'"));
        }, "delete");

    REQUIRE(accounts(sb) == 1);
}


FLUXMT_TEST_CASE("psql live: failed transactions leave no trace", "[psql]") {
    Sandbox sb;
    create_tables(sb);
    seed(sb);

    SECTION("an error result aborts") {
        auto result = flux::psql::try_with_tx(sb.db(), [&](pqxx::work& tx) -> ers::Result<int> {
            tx.exec(sb.sql("INSERT INTO @.account (email) VALUES ('carol.example')"));
            return ers::make_error(flux::EDbError::Other, "changed my mind");
            }, "abort");

        REQUIRE(!result);
        REQUIRE(result.error().code() == flux::EDbError::Other);
        REQUIRE(result.error().detail() == "changed my mind");
    }

    SECTION("an exception aborts") {
        auto result = flux::psql::try_with_tx(sb.db(), [&](pqxx::work& tx) {
            tx.exec(sb.sql("INSERT INTO @.account (email) VALUES ('carol.example')"));
            throw std::runtime_error("boom");
            }, "throw");

        REQUIRE(!result);
        REQUIRE(result.error().code() == flux::EDbError::Other);
    }

    REQUIRE(accounts(sb) == 2);
}


FLUXMT_TEST_CASE("psql live: constraint violations are classified", "[psql]") {
    Sandbox sb;
    create_tables(sb);
    seed(sb);

    auto code_of = [&](std::string statement) -> std::error_code {
        auto result = flux::psql::try_with_tx(sb.db(), [&](pqxx::work& tx) {
            tx.exec(sb.sql(statement));
            }, "constraint");

        return result ? std::error_code{} : result.error().code();
        };

    CHECK(code_of("INSERT INTO @.account (email) VALUES ('alice.example')") == flux::EDbError::UniqueViolation);
    CHECK(code_of("INSERT INTO @.entry (account_id, amount) VALUES (999999, 1)") == flux::EDbError::ForeignKeyViolation);
    CHECK(code_of("UPDATE @.account SET balance = -1 WHERE email = 'alice.example'") == flux::EDbError::CheckViolation);
    CHECK(code_of("INSERT INTO @.account (email) VALUES (NULL)") == flux::EDbError::NotNullViolation);
    CHECK(code_of("SELECT * FROM @.missing") == flux::EDbError::Other);

    REQUIRE(accounts(sb) == 2);
}


FLUXMT_TEST_CASE("psql live: concurrent updates through the pool", "[psql]") {
    Sandbox sb;
    create_tables(sb);
    seed(sb);

    constexpr int threads = 8;
    constexpr int rounds = 25;

    std::atomic_int failures = 0;
    std::vector<std::jthread> workers;

    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&] {
            for (int r = 0; r < rounds; ++r) {
                auto result = flux::psql::try_with_tx(sb.db(), [&](pqxx::work& tx) {
                    tx.exec(sb.sql("UPDATE @.account SET balance = balance + 1 WHERE email = 'alice.example'"));
                    }, flux::psql::tx_options_t{ .label = "increment", .retries = 3 });

                if (!result)
                    ++failures;
            }
            });
    }
    workers.clear();

    REQUIRE(failures == 0);
    REQUIRE(balance_of(sb, "alice.example") == 100 + threads * rounds);
}


FLUXMT_TEST_CASE("psql live: an exhausted pool times out and recovers", "[psql]") {
    Sandbox sb({ .min_size = 1, .max_size = 1, .acquire_timeout = std::chrono::milliseconds(100) });

    {
        auto held = sb.db().acquire();
        REQUIRE(held);

        auto second = sb.db().acquire();
        REQUIRE(!second);
        REQUIRE(second.error().code() == flux::EDbError::PoolTimeout);
    }

    auto again = sb.db().acquire();
    REQUIRE(again);
}
