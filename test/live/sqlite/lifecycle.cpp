// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <chrono>
#include <stdexcept>
#include <thread>

// fluxmt
#include <fluxmt/sqlite.hpp>

// test
#include "support/sqlite/db.hpp"
#include "support/test_case.hpp"


namespace {
    using namespace std::chrono_literals;
    namespace sq = flux::sqlite;

    sq::Database open(const test::TempDb& file, size_t max_size = 2, ers::ms_t busy = 5000ms) {
        auto options = file.options();
        options.busy_timeout = busy;

        return sq::Database(options, { .min_size = 0, .max_size = max_size, .acquire_timeout = 500ms });
    }

    void create_schema(sq::Database& db) {
        sq::with_tx(db, [](sq::transaction& tx) {
            tx.exec(R"(
                CREATE TABLE account (
                    id      INTEGER PRIMARY KEY AUTOINCREMENT,
                    email   TEXT    NOT NULL UNIQUE,
                    balance INTEGER NOT NULL DEFAULT 0 CHECK (balance >= 0)
                )
            )");
            tx.exec(R"(
                CREATE TABLE entry (
                    id         INTEGER PRIMARY KEY,
                    account_id INTEGER NOT NULL REFERENCES account (id),
                    amount     INTEGER NOT NULL
                )
            )");
            }, "schema");
    }

    ers::i64 accounts(sq::Database& db) {
        return sq::with_tx(db, [](sq::transaction& tx) {
            return *tx.exec("SELECT count(*) FROM account").rows.at(0).at(0).get_if<ers::i64>();
            });
    }

    void add(sq::transaction_base& tx, const char* email, ers::i64 balance = 0) {
        tx.exec("INSERT INTO account (email, balance) VALUES (?1, ?2)", { email, balance });
    }
}


FLUXMT_TEST_CASE("sqlite commit makes changes visible", "[sqlite]") {
    test::TempDb file;
    auto db = open(file);
    create_schema(db);

    sq::with_tx(db, [](sq::transaction& tx) { add(tx, "alice"); }, "insert");

    REQUIRE(accounts(db) == 1);
}

FLUXMT_TEST_CASE("sqlite exception rolls back", "[sqlite]") {
    test::TempDb file;
    auto db = open(file);
    create_schema(db);

    REQUIRE_THROWS_AS(sq::with_tx(db, [](sq::transaction& tx) {
        add(tx, "alice");
        throw std::runtime_error("boom");
        }), std::runtime_error);

    REQUIRE(accounts(db) == 0);
}

FLUXMT_TEST_CASE("sqlite Result error aborts", "[sqlite]") {
    test::TempDb file;
    auto db = open(file);
    create_schema(db);

    auto failed = sq::with_tx(db, [](sq::transaction& tx) -> ers::Result<int> {
        add(tx, "alice");
        return ers::make_error(flux::EDbError::Other, "no");
        });

    REQUIRE(!failed);
    REQUIRE(accounts(db) == 0);
}

FLUXMT_TEST_CASE("sqlite constraint violations are classified", "[sqlite]") {
    test::TempDb file;
    auto db = open(file);
    create_schema(db);
    sq::with_tx(db, [](sq::transaction& tx) { add(tx, "alice", 10); });

    auto code_of = [&](auto fn) {
        auto result = sq::try_with_tx(db, fn, "violation");
        REQUIRE(!result);
        return result.error().code();
        };

    REQUIRE(code_of([](sq::transaction& tx) { add(tx, "alice"); }) == flux::EDbError::UniqueViolation);
    REQUIRE(code_of([](sq::transaction& tx) { add(tx, "bob", -1); }) == flux::EDbError::CheckViolation);
    REQUIRE(code_of([](sq::transaction& tx) {
        tx.exec("INSERT INTO account (email, balance) VALUES (NULL, 0)");
        }) == flux::EDbError::NotNullViolation);
    REQUIRE(code_of([](sq::transaction& tx) {
        tx.exec("INSERT INTO entry (account_id, amount) VALUES (999, 1)");
        }) == flux::EDbError::ForeignKeyViolation);

    REQUIRE(accounts(db) == 1);
}

FLUXMT_TEST_CASE("sqlite busy writer is reported", "[sqlite]") {
    test::TempDb file;
    auto db = open(file, 2, 10ms);
    create_schema(db);

    auto holder = db.acquire_or_throw();
    sq::write_transaction lock(*holder, "lock");

    auto blocked = sq::try_with_tx(db, [](sq::write_transaction& tx) { add(tx, "alice"); }, "blocked");

    REQUIRE(!blocked);
    REQUIRE(blocked.error().code() == flux::EDbError::Busy);
}

FLUXMT_TEST_CASE("sqlite retries a busy writer", "[sqlite]") {
    test::TempDb file;
    auto db = open(file, 2, 10ms);
    create_schema(db);

    auto holder = db.acquire_or_throw();
    sq::write_transaction lock(*holder, "lock");

    std::thread release([&] {
        std::this_thread::sleep_for(100ms);
        lock.commit();
        });

    sq::with_tx(db, [](sq::write_transaction& tx) { add(tx, "alice"); },
        flux::tx_options_t{ .label = "retry", .retries = 20, .backoff = 20ms });

    release.join();
    REQUIRE(accounts(db) == 1);
}

FLUXMT_TEST_CASE("sqlite memory databases are private to a connection", "[sqlite]") {
    sq::Database db({}, { .min_size = 0, .max_size = 2 });

    auto first = db.acquire_or_throw();
    auto second = db.acquire_or_throw();

    first->exec_script("CREATE TABLE t (x)");

    REQUIRE_THROWS_AS(second->exec("SELECT * FROM t"), sq::error);
}
