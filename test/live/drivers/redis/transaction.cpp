// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <stdexcept>
#include <string>
#include <vector>

// fluxmt
#include <fluxmt/drivers/redis.hpp>

// test
#include "support/drivers/redis/live.hpp"
#include "support/test_case.hpp"


namespace {
    namespace redis = flux::redis;

    template<typename T>
    T as(const flux::Value& value) {
        return *value.get_if<T>();
    }

    ers::i64 exists(redis::Database& db, const std::string& key) {
        return redis::with_conn(db, [&](redis::connection& conn) {
            return as<ers::i64>(conn.exec({ "EXISTS", key }));
        });
    }

    std::string ping(redis::Database& db) {
        return redis::with_conn(db, [](redis::connection& conn) {
            return as<std::string>(conn.exec({ "PING" }));
        });
    }
}


FLUXMT_TEST_CASE("redis commit returns the queued answers", "[redis]") {
    test::live::Sandbox sandbox;
    const auto key = sandbox.key("counter");

    auto answers = redis::with_tx(sandbox.db(), [&](redis::transaction& tx) {
        tx.queue({ "INCR", key });
        tx.queue({ "INCR", key });
        tx.commit();

        return std::vector(tx.results());
    }, "incr");

    REQUIRE(answers.size() == 2);
    REQUIRE(as<ers::i64>(answers[0]) == 1);
    REQUIRE(as<ers::i64>(answers[1]) == 2);
}

FLUXMT_TEST_CASE("redis Result error discards the queue", "[redis]") {
    test::live::Sandbox sandbox;
    const auto key = sandbox.key("discarded");

    auto failed = redis::with_tx(sandbox.db(), [&](redis::transaction& tx) -> ers::Result<int> {
        tx.queue({ "SET", key, "x" });
        return ers::make_error(flux::EDbError::Other, "no");
    });

    REQUIRE(!failed);
    REQUIRE(exists(sandbox.db(), key) == 0);
}

FLUXMT_TEST_CASE("redis exception leaves the connection usable", "[redis]") {
    test::live::Sandbox sandbox({ .min_size = 1, .max_size = 1 });
    const auto key = sandbox.key("thrown");

    REQUIRE_THROWS_AS(redis::with_tx(sandbox.db(), [&](redis::transaction& tx) {
        tx.queue({ "SET", key, "x" });
        throw std::runtime_error("boom");
    }), std::runtime_error);

    REQUIRE(ping(sandbox.db()) == "PONG");
    REQUIRE(exists(sandbox.db(), key) == 0);
}

FLUXMT_TEST_CASE("redis server rejects a malformed queued command", "[redis]") {
    test::live::Sandbox sandbox({ .min_size = 1, .max_size = 1 });

    REQUIRE_THROWS_AS(redis::with_tx(sandbox.db(), [](redis::transaction& tx) {
        tx.queue({ "INCR" });
    }), redis::error);

    REQUIRE(ping(sandbox.db()) == "PONG");
}

FLUXMT_TEST_CASE("redis exec is closed once a command is queued", "[redis]") {
    test::live::Sandbox sandbox;
    const auto key = sandbox.key("misuse");

    REQUIRE_THROWS_AS(redis::with_tx(sandbox.db(), [&](redis::transaction& tx) {
        tx.queue({ "SET", key, "x" });
        tx.exec({ "GET", key });
    }), std::logic_error);
}

FLUXMT_TEST_CASE("redis watch conflict is a serialization failure", "[redis]") {
    test::live::Sandbox sandbox({ .min_size = 0, .max_size = 2 });
    const auto key = sandbox.key("watched");

    auto other = sandbox.db().acquire_or_throw();

    auto result = redis::try_with_tx(sandbox.db(), [&](redis::transaction& tx) {
        tx.watch(key);
        other->exec({ "SET", key, "theirs" });

        tx.queue({ "SET", key, "mine" });
        tx.commit();
    }, "conflict");

    REQUIRE(!result);
    REQUIRE(result.error().code() == flux::EDbError::SerializationFailure);
    REQUIRE(as<std::string>(other->exec({ "GET", key })) == "theirs");
}

FLUXMT_TEST_CASE("redis retries after a watch conflict", "[redis]") {
    test::live::Sandbox sandbox({ .min_size = 0, .max_size = 2 });
    const auto key = sandbox.key("watched");

    auto other = sandbox.db().acquire_or_throw();
    int attempts = 0;

    redis::with_tx(sandbox.db(), [&](redis::transaction& tx) {
        tx.watch(key);

        if (++attempts == 1)
            other->exec({ "SET", key, "theirs" });

        tx.queue({ "SET", key, "mine" });
    }, flux::tx_options_t { .label = "retry", .retries = 2 });

    REQUIRE(attempts == 2);
    REQUIRE(as<std::string>(other->exec({ "GET", key })) == "mine");
}

FLUXMT_TEST_CASE("redis watch without a queue just unwatches", "[redis]") {
    test::live::Sandbox sandbox({ .min_size = 1, .max_size = 1 });
    const auto key = sandbox.key("read only");

    auto value = redis::with_tx(sandbox.db(), [&](redis::transaction& tx) {
        tx.watch(key);
        return tx.exec({ "GET", key });
    });

    REQUIRE(value.is_null());
    REQUIRE(ping(sandbox.db()) == "PONG");
}
