// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string>

// fluxmt
#include <fluxmt/redis.hpp>

// test
#include "support/redis/live.hpp"
#include "support/test_case.hpp"


namespace {
    std::string ping(flux::redis::connection& conn) {
        return *conn.exec({ "PING" }).get_if<std::string>();
    }
}


FLUXMT_TEST_CASE("redis pool drops a killed connection", "[redis]") {
    test::live::Sandbox sandbox({ .min_size = 0, .max_size = 2 });
    auto& db = sandbox.db();

    {
        auto victim = db.acquire_or_throw();
        auto killer = db.acquire_or_throw();

        const auto id = *victim->exec({ "CLIENT", "ID" }).get_if<ers::i64>();
        killer->exec({ "CLIENT", "KILL", "ID", id });

        REQUIRE_THROWS_AS(victim->exec({ "PING" }), flux::redis::error);
        REQUIRE(!victim->is_open());
    }

    auto first = db.acquire_or_throw();
    auto second = db.acquire_or_throw();

    REQUIRE(ping(*first) == "PONG");
    REQUIRE(ping(*second) == "PONG");
}

FLUXMT_TEST_CASE("redis closed connection reports connection lost", "[redis]") {
    test::live::Sandbox sandbox({ .min_size = 0, .max_size = 2 });
    auto& db = sandbox.db();

    auto victim = db.acquire_or_throw();
    auto killer = db.acquire_or_throw();

    const auto id = *victim->exec({ "CLIENT", "ID" }).get_if<ers::i64>();
    killer->exec({ "CLIENT", "KILL", "ID", id });

    try {
        victim->exec({ "PING" });
        FAIL("expected an error");
    } catch (const flux::redis::error& e) {
        REQUIRE(e.kind() == flux::EDbError::ConnectionLost);
    }

    try {
        victim->exec({ "PING" });
        FAIL("expected an error");
    } catch (const flux::redis::error& e) {
        REQUIRE(e.kind() == flux::EDbError::ConnectionLost);
    }
}
