// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <stdexcept>
#include <string>

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
}


FLUXMT_TEST_CASE("redis ping", "[redis]") {
    test::live::Sandbox sandbox;

    auto reply = flux::with_conn(sandbox.db(), [](redis::connection& conn) {
        return conn.exec({ "PING" });
    });

    REQUIRE(as<std::string>(reply) == "PONG");
}

FLUXMT_TEST_CASE("redis set get and counters", "[redis]") {
    test::live::Sandbox sandbox;
    const auto greeting = sandbox.key("greeting");
    const auto counter = sandbox.key("counter");

    flux::with_conn(sandbox.db(), [&](redis::connection& conn) {
        REQUIRE(as<std::string>(conn.exec({ "SET", greeting, "hello" })) == "OK");
        REQUIRE(as<std::string>(conn.exec({ "GET", greeting })) == "hello");

        REQUIRE(as<ers::i64>(conn.exec({ "INCR", counter })) == 1);
        REQUIRE(as<ers::i64>(conn.exec({ "INCRBY", counter, 41 })) == 42);

        REQUIRE(conn.exec({ "GET", sandbox.key("missing") }).is_null());
    });
}

FLUXMT_TEST_CASE("redis values are binary safe", "[redis]") {
    test::live::Sandbox sandbox;
    const auto key = sandbox.key("blob");
    const flux::value_bytes_t blob { std::byte { 1 }, std::byte { 0 }, std::byte { 255 }, std::byte { 'z' } };

    flux::with_conn(sandbox.db(), [&](redis::connection& conn) {
        conn.exec({ "SET", key, blob });

        const auto& stored = as<std::string>(conn.exec({ "GET", key }));
        REQUIRE(stored.size() == blob.size());
        REQUIRE(stored == std::string("\x01\0\xff" "z", 4));
    });
}

FLUXMT_TEST_CASE("redis arrays come back nested", "[redis]") {
    test::live::Sandbox sandbox;
    const auto key = sandbox.key("hash");

    flux::with_conn(sandbox.db(), [&](redis::connection& conn) {
        conn.exec({ "HSET", key, "a", 1, "b", 2 });

        const auto& fields = as<flux::value_array_t>(conn.exec({ "HMGET", key, "a", "b", "missing" }));

        REQUIRE(fields.size() == 3);
        REQUIRE(as<std::string>(fields[0]) == "1");
        REQUIRE(as<std::string>(fields[1]) == "2");
        REQUIRE(fields[2].is_null());
    });
}

FLUXMT_TEST_CASE("redis server errors become typed errors", "[redis]") {
    test::live::Sandbox sandbox;
    const auto key = sandbox.key("list");

    auto failed = flux::try_with_conn(sandbox.db(), [&](redis::connection& conn) {
        conn.exec({ "LPUSH", key, "x" });
        return conn.exec({ "GET", key });
    });

    REQUIRE(!failed);
    REQUIRE(failed.error().message().contains("WRONGTYPE"));
}

FLUXMT_TEST_CASE("redis refuses a null argument before sending", "[redis]") {
    test::live::Sandbox sandbox;
    const auto key = sandbox.key("null");

    flux::with_conn(sandbox.db(), [&](redis::connection& conn) {
        REQUIRE_THROWS_AS(conn.exec({ "SET", key, flux::Value{} }), std::invalid_argument);
        REQUIRE(as<std::string>(conn.exec({ "PING" })) == "PONG");
    });
}
