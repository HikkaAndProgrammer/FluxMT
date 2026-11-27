// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string_view>

// fluxmt
#include <fluxmt/core.hpp>

// test
#include "support/test_case.hpp"


// Details ---------------------------------------------------------------------

namespace {
    struct fake_backend {};

    struct not_a_backend {};
}

template<>
struct flux::ext::backend_traits<fake_backend> {
    static constexpr std::string_view name = "fake";
    static constexpr feature_set features { EFeature::Sql, EFeature::Returning };

    using connection = int;
};


// Tests -----------------------------------------------------------------------

FLUXMT_TEST_CASE("backend traits", "[core]") {
    STATIC_REQUIRE(flux::IsBackend<fake_backend>);
    STATIC_REQUIRE(!flux::IsBackend<not_a_backend>);

    STATIC_REQUIRE(flux::SupportsFeature<fake_backend, flux::EFeature::Returning>);
    STATIC_REQUIRE(!flux::SupportsFeature<fake_backend, flux::EFeature::Upsert>);

    STATIC_REQUIRE(std::same_as<flux::connection_t<fake_backend>, int>);
}

FLUXMT_TEST_CASE("feature set", "[core]") {
    constexpr flux::feature_set none {};
    constexpr flux::feature_set some { flux::EFeature::Sql, flux::EFeature::Upsert };

    STATIC_REQUIRE(none.bits == 0);
    STATIC_REQUIRE(!none.contains(flux::EFeature::Sql));

    STATIC_REQUIRE(some.contains(flux::EFeature::Sql));
    STATIC_REQUIRE(some.contains(flux::EFeature::Upsert));
    STATIC_REQUIRE(!some.contains(flux::EFeature::Returning));
}

FLUXMT_TEST_CASE("library version matches headers", "[core]") {
    REQUIRE(flux::library_version() == flux::version);
}
