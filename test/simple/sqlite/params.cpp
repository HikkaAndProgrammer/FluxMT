// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <chrono>
#include <stdexcept>
#include <string>

// fluxmt
#include <fluxmt/sqlite.hpp>

// test
#include "support/test_case.hpp"


namespace {
    flux::Value roundtrip(flux::Value value) {
        flux::impl::sqlite::connection conn(flux::sqlite::options_t{});
        return conn.exec("SELECT ?1", { std::move(value) }).rows.at(0).at(0);
    }

    template<typename T>
    const T& as(const flux::Value& value) {
        return *value.get_if<T>();
    }
}


FLUXMT_TEST_CASE("sqlite scalar round trip", "[sqlite]") {
    REQUIRE(roundtrip(flux::Value{}).is_null());
    REQUIRE(as<ers::i64>(roundtrip(true)) == 1);
    REQUIRE(as<ers::i64>(roundtrip(ers::i64{ -7 })) == -7);
    REQUIRE(as<ers::f64>(roundtrip(1.5)) == 1.5);
    REQUIRE(as<std::string>(roundtrip(std::string("hello"))) == "hello");
}

FLUXMT_TEST_CASE("sqlite blob round trip", "[sqlite]") {
    const flux::value_bytes_t blob{ std::byte{ 1 }, std::byte{ 0 }, std::byte{ 255 } };

    REQUIRE(as<flux::value_bytes_t>(roundtrip(blob)) == blob);
    REQUIRE(as<flux::value_bytes_t>(roundtrip(flux::value_bytes_t{})).empty());
}

FLUXMT_TEST_CASE("sqlite time and uuid are stored as text", "[sqlite]") {
    const auto day = std::chrono::sys_days{ std::chrono::year(2026) / 11 / 24 };
    const ers::timestamp_t moment = day + std::chrono::hours(12) + std::chrono::minutes(30);
    const ers::uuid id = ers::uuid_from_string("123e4567-e89b-12d3-a456-426614174000");

    REQUIRE(as<std::string>(roundtrip(ers::date_t{ day })) == "2026-11-24");
    REQUIRE(as<std::string>(roundtrip(moment)) == "2026-11-24 12:30:00.000000");
    REQUIRE(as<std::string>(roundtrip(id)) == ers::uuid_to_string(id));
}

FLUXMT_TEST_CASE("sqlite rejects what it cannot store", "[sqlite]") {
    REQUIRE_THROWS_AS(roundtrip(ers::u64{ 1 } << 63), std::out_of_range);
    REQUIRE_THROWS_AS(roundtrip(flux::value_array_t{ 1, 2 }), std::invalid_argument);
    REQUIRE(as<ers::i64>(roundtrip(ers::u64{ 42 })) == 42);
}
