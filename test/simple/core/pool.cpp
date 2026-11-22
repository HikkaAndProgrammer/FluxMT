// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <chrono>

// fluxmt
#include <fluxmt/core.hpp>

// test
#include "support/fake_backend.hpp"
#include "support/test_case.hpp"


using pool_t = flux::Pool<test::fake_backend>;


namespace {
    flux::pool_options_t quick(size_t max_size) {
        return { .min_size = 0, .max_size = max_size, .acquire_timeout = std::chrono::milliseconds(20) };
    }
}


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
