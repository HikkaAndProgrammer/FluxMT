// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <chrono>
#include <string>
#include <thread>
#include <vector>

// fluxmt
#include <fluxmt/core.hpp>

// test
#include "support/fake_backend.hpp"
#include "support/test_case.hpp"


using namespace std::chrono_literals;

using pool_t = flux::Pool<test::fake_backend>;


// Details ---------------------------------------------------------------------

namespace {
    flux::pool_options_t quick(size_t max_size) {
        return {
            .min_size        = 0,
            .max_size        = max_size,
            .acquire_timeout = std::chrono::milliseconds(20)
        };
    }

    flux::pool_options_t aging(size_t min_size, size_t max_size) {
        return {
            .min_size        = min_size,
            .max_size        = max_size,
            .acquire_timeout = 20ms,
            .idle_timeout    = 1ms
        };
    }

    void age() {
        std::this_thread::sleep_for(5ms);
    }

    // A new connection has an empty log, so a mark tells it from a replacement.
    void mark(test::fake_connection& conn) {
        conn.log.push_back("mine");
    }

    const std::vector<std::string> marked { "mine" };
}


// Leasing ---------------------------------------------------------------------

FLUXMT_TEST_CASE("pool reuses a released connection", "[core]") {
    pool_t pool({}, quick(1));

    test::fake_connection* first = nullptr;
    {
        auto lease = pool.acquire_or_throw();
        first = &*lease;
    }

    auto lease = pool.acquire_or_throw();
    REQUIRE(&*lease == first);
}

FLUXMT_TEST_CASE("pool times out when exhausted", "[core]") {
    pool_t pool({}, quick(1));

    auto held = pool.acquire_or_throw();
    auto second = pool.acquire();

    REQUIRE(!second);
    REQUIRE(second.error().code() == flux::EDbError::PoolTimeout);
}

FLUXMT_TEST_CASE("pool hands out up to max_size", "[core]") {
    pool_t pool({}, quick(2));

    auto a = pool.acquire_or_throw();
    auto b = pool.acquire_or_throw();

    REQUIRE(&*a != &*b);
    REQUIRE(!pool.acquire());
}

FLUXMT_TEST_CASE("pool replaces a dead connection", "[core]") {
    pool_t pool({}, quick(1));

    {
        auto lease = pool.acquire_or_throw();
        lease->open = false;
    }

    auto lease = pool.acquire_or_throw();
    REQUIRE(lease->open);
}

FLUXMT_TEST_CASE("pool reports a failed open as an error", "[core]") {
    pool_t pool({ .fail = true }, quick(1));

    auto lease = pool.acquire();
    REQUIRE(!lease);
}

FLUXMT_TEST_CASE("pool copies share one state", "[core]") {
    pool_t pool({}, quick(1));
    pool_t copy = pool;

    auto held = pool.acquire_or_throw();
    REQUIRE(!copy.acquire());
}


// Idle expiry -----------------------------------------------------------------

FLUXMT_TEST_CASE("pool keeps min_size connections past idle_timeout", "[core]") {
    pool_t pool({}, aging(1, 2));

    {
        auto lease = pool.acquire_or_throw();
        mark(*lease);
    }

    age();

    auto lease = pool.acquire_or_throw();
    REQUIRE(lease->log == marked);
}

FLUXMT_TEST_CASE("pool drops an idle connection above min_size", "[core]") {
    pool_t pool({}, aging(0, 1));

    {
        auto lease = pool.acquire_or_throw();
        mark(*lease);
    }

    age();

    auto lease = pool.acquire_or_throw();
    REQUIRE(lease->log.empty());
}

FLUXMT_TEST_CASE("pool maintain keeps min_size connections", "[core]") {
    pool_t pool({}, aging(1, 2));

    {
        auto a = pool.acquire_or_throw();
        auto b = pool.acquire_or_throw();

        mark(*a);
        mark(*b);
    }

    age();
    pool.maintain();

    auto a = pool.acquire_or_throw();
    auto b = pool.acquire_or_throw();

    REQUIRE(a->log.size() + b->log.size() == 1);
}
