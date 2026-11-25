// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// fluxmt
#include <fluxmt/core.hpp>

// test
#include "support/test_case.hpp"


namespace {
    enum class Color { Red = 2 };

    struct Money {
        int cents;
    };
}

template<>
struct flux::impl::value_traits<Money> {
    static flux::Value to_value(const Money& what) {
        return flux::custom_value_t { "money", std::to_string(what.cents) };
    }
};


FLUXMT_TEST_CASE("value traits numbers", "[core]") {
    REQUIRE(flux::to_value(5) == flux::Value(ers::i64{ 5 }));
    REQUIRE(flux::to_value(short{ 3 }) == flux::Value(ers::i64{ 3 }));
    REQUIRE(flux::to_value(5u) == flux::Value(ers::u64{ 5 }));
    REQUIRE(flux::to_value(ers::u64{ 7 }) == flux::Value(ers::u64{ 7 }));
    REQUIRE(flux::to_value(1.5f) == flux::Value(ers::f64{ 1.5 }));
    REQUIRE(flux::to_value(true) == flux::Value(true));
    REQUIRE(flux::to_value(Color::Red) == flux::Value(ers::i64{ 2 }));
}

FLUXMT_TEST_CASE("value traits text and bytes", "[core]") {
    const std::string expected = "text";

    REQUIRE(flux::to_value("text") == flux::Value(expected));
    REQUIRE(flux::to_value(std::string_view("text")) == flux::Value(expected));
    REQUIRE(flux::to_value(std::string("text")) == flux::Value(expected));

    const flux::value_bytes_t blob{ std::byte{ 1 }, std::byte{ 0 } };
    REQUIRE(flux::to_value(blob) == flux::Value(blob));
}

FLUXMT_TEST_CASE("value traits arrays and optionals", "[core]") {
    REQUIRE(flux::to_value(std::vector<int>{ 1, 2 })
        == flux::Value(flux::value_array_t{ ers::i64{ 1 }, ers::i64{ 2 } }));
    REQUIRE(flux::to_value(std::vector<int>{}) == flux::Value(flux::value_array_t{}));

    const auto nested = flux::to_value(std::vector<std::vector<int>>{ { 1 }, {} });
    REQUIRE(nested.get_if<flux::value_array_t>()->size() == 2);

    REQUIRE(flux::to_value(std::optional<int>{}).is_null());
    REQUIRE(flux::to_value(std::optional<int>{ 7 }) == flux::Value(ers::i64{ 7 }));
}

FLUXMT_TEST_CASE("value traits are extensible", "[core]") {
    REQUIRE(flux::to_value(Money{ 250 }) == flux::Value(flux::custom_value_t { "money", "250" }));
    REQUIRE(flux::to_value(flux::Value(ers::i64{ 9 })) == flux::Value(ers::i64{ 9 }));
}
