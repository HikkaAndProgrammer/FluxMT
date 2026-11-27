// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <array>
#include <concepts>
#include <string_view>

// fluxmt
#include <fluxmt/drivers/shared.hpp>
#include <fluxmt/sql.hpp>

// test
#include "support/sql/backend.hpp"
#include "support/sql/dialect.hpp"
#include "support/sql/orm.hpp"
#include "support/test_case.hpp"


using namespace flux::orm;


// Details ---------------------------------------------------------------------

namespace {
    struct lean_backend {};

    constexpr flux::ext::slot_info_t lean_select[] = {
        flux::impl::shared::select_keyword,
        flux::impl::shared::select_columns,
        flux::impl::shared::from,
        flux::impl::shared::where,
    };

    constexpr flux::sql::syntax_t lean_syntax {};


    class LeanDialect final : public flux::sql::Dialect {
    public:
        LeanDialect() noexcept : SqlDialect(lean_syntax) {}

        std::string_view name() const noexcept override { return "lean"; }
        std::string placeholder(size_t index) const override { return "?" + std::to_string(index); }
        bool is_reserved(std::string_view) const noexcept override { return false; }
    };
}

template<>
struct flux::impl::backend_traits<lean_backend> {
    using connection = int;

    static constexpr std::string_view name = "lean";
    static constexpr feature_set features { EFeature::Sql };

    static constexpr const syntax_t& syntax = lean_syntax;

    static const SqlDialect& dialect() noexcept {
        static const LeanDialect instance;
        return instance;
    }
};


// Tests -----------------------------------------------------------------------

FLUXMT_TEST_CASE("builder collects clauses and renders", "[sql]") {
    auto q = test::anti::orm::select_from("users")
        | where(col("a") == 1)
        | order_by("id")
        | with_limit(3);

    const auto r = q.to_statement();

    REQUIRE(r);
    CHECK(r->text == "SELECT *\nFROM users\nWHERE a = ?1\nORDER BY id ASC\nLIMIT ?2");
}

FLUXMT_TEST_CASE("builder keeps its type while clauses are added", "[sql]") {
    using anti = test::anti_backend;

    STATIC_REQUIRE(std::same_as<
        decltype(test::anti::orm::select_from("t") | where(col("a") == 1)),
        flux::sql::QueryBuilder<anti, test::anti::select_layout>
        >);
}

FLUXMT_TEST_CASE("builder takes conditional clauses", "[sql]") {
    auto q = test::anti::orm::select_from("t")
        | when(false, where(col("a") == 1))
        | when(true, where(col("b") == 2));

    CHECK(q.to_statement()->text == "SELECT *\nFROM t\nWHERE b = ?1");
}

FLUXMT_TEST_CASE("builder hands out the plain query", "[sql]") {
    auto q = test::anti::orm::select_from("t");

    CHECK(q.erased().layout());

    const flux::sql::Query plain = std::move(q);

    CHECK(plain.layout());
}

FLUXMT_TEST_CASE("builder takes its layout from the one it was given", "[sql]") {
    const auto q = test::anti::orm::select_from("t");

    const auto layout = q.erased().layout();

    REQUIRE(layout);
    CHECK((*layout).contains(&flux::slots::offset));

    const auto lean = (flux::sql::QueryBuilder<lean_backend, lean_select>() | from("t")).erased().layout();

    REQUIRE(lean);
    CHECK_FALSE((*lean).contains(&flux::slots::offset));
}

FLUXMT_TEST_CASE("builders accept only the clauses their layout and backend can run", "[sql]") {
    using flux::impl::AcceptsClause;

    STATIC_REQUIRE(AcceptsClause<test::anti_backend, test::anti::insert_layout, decltype(returning("id"))>);

    STATIC_REQUIRE(AcceptsClause<lean_backend, lean_select, decltype(where(col("a") == 1))>);
    STATIC_REQUIRE(!AcceptsClause<lean_backend, lean_select, decltype(with_limit(1))>);
    STATIC_REQUIRE(!AcceptsClause<lean_backend, lean_select, decltype(where(col("a").ilike("x")))>);
}
