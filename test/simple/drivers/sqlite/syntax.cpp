// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <vector>

// fluxmt
#include <fluxmt/drivers/sqlite.hpp>
#include <fluxmt/sql.hpp>

// test
#include "support/test_case.hpp"


using namespace flux::orm;


namespace {
    using flux::sql::ESyntax;
    using flux::sql::missing;
    using flux::sql::needs_of;
    using flux::sql::supports;

    constexpr auto& lite = flux::impl::sqlite::sqlite_syntax;

    template<typename E>
    inline constexpr flux::sql::needs_t needs = needs_of<E>();
}


static_assert(supports(lite, needs<decltype(col("a").ilike("x%"))>));
static_assert(supports(lite, needs<decltype(col("a").any(std::vector<int>{}))>));
static_assert(supports(lite, needs<decltype(col("a").cast("text") == col("b"))>));
static_assert(supports(lite, flux::impl::need_syntax(ESyntax::Excluded)));

static_assert(supports(lite, needs<decltype(now())>));
static_assert(supports(lite, needs<decltype(greatest(col("a"), col("b")))>));
static_assert(supports(lite, needs<decltype(least(col("a"), col("b"), 3))>));
static_assert(supports(lite, needs<decltype(coalesce(col("a"), 0))>));

static_assert(!supports(lite, needs<decltype(now(1))>));
static_assert(!supports(lite, needs<decltype(greatest(col("a")))>));
static_assert(!supports(lite, needs<decltype(least(col("a")))>));
static_assert(!supports(lite, needs<decltype(col("a") > 1 && greatest(col("b")) == 2)>));

static_assert(missing(lite, needs<decltype(greatest(col("a")))>) == flux::impl::need_function("greatest", 1));


FLUXMT_TEST_CASE("sqlite dialect is built on its syntax table", "[sqlite]") {
    const auto& syntax = flux::sqlite::dialect().syntax();

    CHECK(syntax.cast == lite.cast);
    CHECK(syntax.ilike == lite.ilike);
    CHECK(syntax.any == lite.any);
    CHECK(syntax.excluded == lite.excluded);
    CHECK(syntax.functions.data() == lite.functions.data());
    CHECK(syntax.function == nullptr);
}

FLUXMT_TEST_CASE("sqlite spells functions at compile time and at render time the same way", "[sqlite]") {
    constexpr auto now_with_argument = flux::impl::spelling_of(lite, "now", 1);
    constexpr auto one_greatest = flux::impl::spelling_of(lite, "greatest", 1);
    constexpr auto two_greatest = flux::impl::spelling_of(lite, "greatest", 2);
    constexpr auto unknown = flux::impl::spelling_of(lite, "coalesce", 1);

    CHECK_FALSE(now_with_argument.supported);
    CHECK_FALSE(one_greatest.supported);
    CHECK(two_greatest.supported);
    CHECK(two_greatest.name == "max");
    CHECK(unknown.supported);
    CHECK(unknown.name.empty());
}
