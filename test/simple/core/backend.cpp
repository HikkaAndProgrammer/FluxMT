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
struct flux::backend_traits<fake_backend> {
    static constexpr std::string_view name = "fake";
    static constexpr feature_set features { EFeature::Sql, EFeature::Returning };

    using connection = int;
};


// Tests -----------------------------------------------------------------------

FLUXMT_TEST_CASE("backend traits", "[core]") {
    STATIC_REQUIRE(flux::Backend<fake_backend>);
    STATIC_REQUIRE(!flux::Backend<not_a_backend>);

    STATIC_REQUIRE(flux::Supports<fake_backend, flux::EFeature::Returning>);
    STATIC_REQUIRE(!flux::Supports<fake_backend, flux::EFeature::Upsert>);
}

FLUXMT_TEST_CASE("library version matches headers", "[core]") {
    REQUIRE(flux::library_version() == flux::version);
}
