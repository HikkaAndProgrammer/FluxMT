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

    constexpr auto lean_layouts = std::to_array<flux::sql::layout_entry_t>({
        { .kind = &flux::kinds::select, .layout = lean_select },
    });

    constexpr flux::sql::syntax_t lean_syntax {};


    class LeanDialect final : public flux::sql::Dialect {
    public:
        LeanDialect() noexcept : SqlDialect(lean_syntax) {}

        std::string_view name() const noexcept override { return "lean"; }
        std::string placeholder(size_t index) const override { return "?" + std::to_string(index); }
        bool is_reserved(std::string_view) const noexcept override { return false; }

        flux::ext::slot_info_view layout(flux::ext::kind_ref kind) const noexcept override {
            return flux::ext::find_layout(lean_layouts, kind);
        }
    };
}

template<>
struct flux::impl::backend_traits<lean_backend> {
    using connection = int;

    static constexpr std::string_view name = "lean";
    static constexpr feature_set features { EFeature::Sql };

    static constexpr const syntax_t& syntax = lean_syntax;

    static constexpr slot_info_view layout(kind_ref kind) noexcept {
        return find_layout(lean_layouts, kind);
    }

    static const SqlDialect& dialect() noexcept {
        static const LeanDialect instance;
        return instance;
    }
};


// Tests -----------------------------------------------------------------------

FLUXMT_TEST_CASE("builder collects clauses and renders", "[sql]") {
    auto q = flux::builders::select_from<test::anti_backend>("users")
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
        decltype(flux::builders::select_from<anti>("t") | where(col("a") == 1)),
        flux::sql::QueryBuilder<anti, &flux::kinds::select>
        >);
}

FLUXMT_TEST_CASE("builder takes conditional clauses", "[sql]") {
    auto q = flux::builders::select_from<test::anti_backend>("t")
        | when(false, where(col("a") == 1))
        | when(true, where(col("b") == 2));

    CHECK(q.to_statement()->text == "SELECT *\nFROM t\nWHERE b = ?1");
}

FLUXMT_TEST_CASE("builder hands out the plain query", "[sql]") {
    auto q = flux::builders::select_from<test::anti_backend>("t");

    CHECK(q.erased().kind() == &flux::kinds::select);

    auto plain = std::move(q).erased();
    plain |= where(col("a") == 1);

    CHECK(plain.to_statement(flux::impl::backend_traits<test::anti_backend>::dialect())->text == "SELECT *\nFROM t\nWHERE a = ?1");
}

FLUXMT_TEST_CASE("backends accept only the clauses they can run", "[sql]") {
    using flux::impl::AcceptsClause;
    using flux::impl::HasKind;

    STATIC_REQUIRE(AcceptsClause<test::anti_backend, &flux::kinds::insert, decltype(returning("id"))>);

    STATIC_REQUIRE(HasKind<lean_backend, &flux::kinds::select>);
    STATIC_REQUIRE(!HasKind<lean_backend, &flux::kinds::insert>);

    STATIC_REQUIRE(AcceptsClause<lean_backend, &flux::kinds::select, decltype(where(col("a") == 1))>);
    STATIC_REQUIRE(!AcceptsClause<lean_backend, &flux::kinds::select, decltype(with_limit(1))>);
    STATIC_REQUIRE(!AcceptsClause<lean_backend, &flux::kinds::select, decltype(where(col("a").ilike("x")))>);
}
