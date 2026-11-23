// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <chrono>
#include <string>
#include <vector>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/core.hpp>

// test
#include "support/fake_backend.hpp"
#include "support/test_case.hpp"


using backend = test::fake_backend;
using log_t = std::vector<std::string>;
using wrapped_t = flux::Tx<backend, test::fake_tx>;


namespace {
    constexpr flux::tx_options_t retry(ers::u8 times) {
        return { .retries = times, .backoff = std::chrono::milliseconds(1) };
    }
}


FLUXMT_TEST_CASE("commit on success, whichever way the callable takes its transaction", "[core]") {
    test::fake_connection conn;

    flux::with_tx_fn<backend>(conn, [](test::fake_tx&) {});
    flux::with_tx_fn<backend>(conn, [](wrapped_t&) {});
    flux::with_tx_fn<backend>(conn, [](auto&) {});

    REQUIRE(conn.log == log_t{ "begin", "commit", "begin", "commit", "begin", "commit" });
}

FLUXMT_TEST_CASE("the result of the callable is returned", "[core]") {
    test::fake_connection conn;

    REQUIRE(flux::with_tx_fn<backend>(conn, [](test::fake_tx&) { return 7; }) == 7);
}

FLUXMT_TEST_CASE("an exception rolls back and propagates", "[core]") {
    test::fake_connection conn;

    REQUIRE_THROWS_AS(
        flux::with_tx_fn<backend>(conn, [](test::fake_tx&) { throw std::runtime_error("x"); }),
        std::runtime_error
    );
    REQUIRE(conn.log == log_t{ "begin", "rollback" });
}

FLUXMT_TEST_CASE("a Result error aborts", "[core]") {
    test::fake_connection conn;

    auto failed = flux::with_tx_fn<backend>(conn, [](test::fake_tx&) -> ers::Result<int> {
        return ers::make_error(flux::EDbError::Other, "no");
    });
    REQUIRE(!failed);
    REQUIRE(conn.log == log_t{ "begin", "abort" });

    conn.log.clear();

    auto fine = flux::with_tx_fn<backend>(conn, [](test::fake_tx&) -> ers::Result<int> { return 5; });
    REQUIRE(fine);
    REQUIRE(*fine == 5);
    REQUIRE(conn.log == log_t{ "begin", "commit" });
}

FLUXMT_TEST_CASE("retry repeats on retryable errors only", "[core]") {
    test::fake_connection conn;
    int calls = 0;

    auto flaky = [&](test::fake_tx&) {
        if (++calls < 3)
            throw test::fake_retryable("deadlock");
        return calls;
    };

    REQUIRE(flux::with_tx_fn<backend>(conn, flaky, retry(2)) == 3);
    REQUIRE(conn.log == log_t{ "begin", "rollback", "begin", "rollback", "begin", "commit" });

    conn.log.clear();
    calls = 0;
    REQUIRE_THROWS_AS(flux::with_tx_fn<backend>(conn, flaky, retry(1)), test::fake_retryable);
    REQUIRE(calls == 2);

    calls = 0;
    REQUIRE_THROWS_AS(
        flux::with_tx_fn<backend>(conn, [&](test::fake_tx&) { ++calls; throw std::runtime_error("x"); }, retry(5)),
        std::runtime_error
    );
    REQUIRE(calls == 1);
}

FLUXMT_TEST_CASE("pool level with_tx and with_conn", "[core]") {
    flux::Pool<backend> pool({}, { .min_size = 0, .max_size = 1 });

    REQUIRE(flux::with_tx(pool, [](test::fake_tx&) { return 1; }, "label") == 1);
    REQUIRE(flux::with_conn(pool, [](test::fake_connection& conn) { return conn.log.size(); }) == 2);
}

FLUXMT_TEST_CASE("try_with_tx turns an exception into a coded diagnostic", "[core]") {
    flux::Pool<backend> pool({}, { .min_size = 0, .max_size = 1 });

    auto result = flux::try_with_tx(pool, [](test::fake_tx&) -> int { throw test::fake_retryable("boom"); }, "job");

    REQUIRE(!result);
    REQUIRE(result.error().code() == flux::EDbError::Deadlock);

    auto fine = flux::try_with_tx(pool, [](test::fake_tx&) { return 2; });
    REQUIRE(fine);
    REQUIRE(*fine == 2);
}

FLUXMT_TEST_CASE("try_with_tx reports a pool failure", "[core]") {
    flux::Pool<backend> pool({}, { .min_size = 0, .max_size = 1, .acquire_timeout = std::chrono::milliseconds(10) });

    auto held = pool.acquire_or_throw();
    auto result = flux::try_with_tx(pool, [](test::fake_tx&) { return 1; });

    REQUIRE(!result);
    REQUIRE(result.error().code() == flux::EDbError::PoolTimeout);
}
