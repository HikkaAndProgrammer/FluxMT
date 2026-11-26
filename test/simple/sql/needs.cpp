// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/sql.hpp>

// test
#include "support/sql/dialect.hpp"
#include "support/test_case.hpp"


using namespace flux::orm;


// Details ---------------------------------------------------------------------

namespace {
    using flux::sql::ESyntax;
    using flux::sql::Expr;
    using flux::sql::TypedExpr;
    using flux::sql::missing;
    using flux::sql::needs_of;
    using flux::sql::no_needs;
    using flux::sql::supports;
    using flux::sql::syntax_t;

    using Plain = TypedExpr<no_needs>;

    template<typename E>
    inline constexpr flux::sql::needs_t needs = needs_of<E>();

    constexpr flux::sql::needs_t only(ESyntax what) {
        return flux::impl::need_syntax(what);
    }

    constexpr flux::sql::function_spelling_t spell(std::string_view name, size_t arity) noexcept {
        if (name == "greatest" && arity < 2)
            return { .supported = false, .reason = "needs two arguments" };

        if (name == "forbidden")
            return { .supported = false };

        return {};
    }

    constexpr syntax_t picky { .function = &spell };

    constexpr flux::sql::function_rule_t pair_rule { .name = "pair", .min_arity = 2, .max_arity = 2, .spelling = { .name = "pair_of" } };
    constexpr flux::sql::function_rule_t pair_mine { .name = "pair", .spelling = { .name = "mine" } };
    constexpr flux::sql::function_rules_t pair_rules { pair_rule };
    constexpr flux::sql::function_rules_t pair_both { pair_mine, pair_rule };
    constexpr flux::sql::function_rules_t pair_added = pair_rules + flux::sql::function_rules_t { pair_mine };

    constexpr syntax_t tabled = flux::sql::with_functions(syntax_t {}, pair_rules);
    constexpr syntax_t tabled_and_picky = flux::sql::with_functions(picky, pair_rules);
    constexpr syntax_t mine_first = flux::sql::with_functions(syntax_t {}, pair_both);
    constexpr syntax_t mine_last = flux::sql::with_functions(syntax_t {}, pair_added);
    constexpr syntax_t nothing {};
    constexpr auto& anti = test::anti::syntax;

    constexpr flux::ext::named_fn<"forbidden"> forbidden {};
    constexpr flux::ext::named_fn<"pair"> pair {};
    constexpr flux::ext::named_fn<"f1"> f1 {};
    constexpr flux::ext::named_fn<"f2"> f2 {};
    constexpr flux::ext::named_fn<"f3"> f3 {};
    constexpr flux::ext::named_fn<"f4"> f4 {};
    constexpr flux::ext::named_fn<"f5"> f5 {};
    constexpr flux::ext::named_fn<"a_function_with_a_name_too_long"> too_long {};


    class BareDialect final : public flux::sql::Dialect {
    public:
        BareDialect() noexcept : SqlDialect(nothing) {}

        std::string_view name() const noexcept override { return "bare"; }
        std::string placeholder(size_t index) const override { return "?" + std::to_string(index); }
        bool is_reserved(std::string_view) const noexcept override { return false; }
        flux::ext::slot_info_view layout(flux::ext::kind_ref) const noexcept override { return {}; }
    };
}


// Types -----------------------------------------------------------------------

static_assert(std::same_as<decltype(col("a")), Plain>);
static_assert(std::same_as<decltype(col("t", "a")), Plain>);
static_assert(std::same_as<decltype(null()), Plain>);
static_assert(std::same_as<decltype(sql_bool(true)), Plain>);
static_assert(std::same_as<decltype(val(5)), Plain>);
static_assert(std::same_as<decltype(param(5)), Plain>);
static_assert(std::same_as<decltype(row(col("a"), 1)), Plain>);

static_assert(std::same_as<decltype(col("a") == 1), Plain>);
static_assert(std::same_as<decltype(5 <= col("a")), Plain>);
static_assert(std::same_as<decltype(col("a") + col("b") * 2), Plain>);
static_assert(std::same_as<decltype((col("a") == 1 && col("b") == 2) || !(col("c") == 3)), Plain>);
static_assert(std::same_as<decltype(col("a").is_null()), Plain>);
static_assert(std::same_as<decltype(col("a").between(1, col("b"))), Plain>);
static_assert(std::same_as<decltype(col("a").in({ 1, 2, 3 })), Plain>);
static_assert(std::same_as<decltype(col("a").in({ col("b"), col("c") })), Plain>);
static_assert(std::same_as<decltype(col("a").in(std::vector<int>{})), Plain>);
static_assert(std::same_as<decltype(count_all()), Plain>);
static_assert(std::same_as<decltype(count_of("a") > 1), Plain>);
static_assert(std::same_as<decltype(sql("a {}", col("b"))), Plain>);

// What is not known stays erased
static_assert(std::same_as<decltype(std::declval<Expr>() == 1), Expr>);
static_assert(std::same_as<decltype(col("a") == std::declval<Expr>()), Expr>);
static_assert(std::same_as<decltype(col("a").in({ col("b"), 1 })), Expr>);
static_assert(std::same_as<decltype(col("a").in(std::declval<flux::sql::Query>())), Expr>);
static_assert(std::same_as<decltype(exists(std::declval<flux::sql::Query>())), Expr>);
static_assert(std::same_as<decltype(percentile_cont(0.5, { "a" })), Expr>);


// Needs -----------------------------------------------------------------------

static_assert(needs<decltype(col("a") == 1)> == no_needs);

static_assert(needs<decltype(col("a").ilike("x"))> == only(ESyntax::Ilike));
static_assert(needs<decltype(col("a").any(std::vector<int>{}))> == only(ESyntax::Any));
static_assert(needs<decltype(col("a").cast("int"))> == only(ESyntax::Cast));

static_assert(needs<decltype(col("a").ilike("x") && col("b").ilike("y"))> == only(ESyntax::Ilike));
static_assert(needs<decltype(col("a").ilike("x") && col("b").cast("int") == 1)>
    == (only(ESyntax::Ilike) | only(ESyntax::Cast)));
static_assert(needs<decltype(!col("a").ilike("x"))> == only(ESyntax::Ilike));
static_assert(needs<decltype(col("a") == col("b").cast("int"))> == only(ESyntax::Cast));
static_assert(needs<decltype(col("a").in({ col("b").cast("int"), col("c").cast("int") }))> == only(ESyntax::Cast));
static_assert(needs<decltype(max_of(col("a").cast("int")))> == only(ESyntax::Cast));

static_assert(needs<decltype(now())> == flux::impl::need_function("now", 0));
static_assert(needs<decltype(greatest(col("a"), col("b")))> == flux::impl::need_function("greatest", 2));
static_assert(needs<decltype(greatest(col("a").cast("int"), 1))>
    == (flux::impl::need_function("greatest", 2) | only(ESyntax::Cast)));
static_assert(needs<decltype(f1(1) + f1(1, 2))>.count == 2);
static_assert(needs<decltype(f1(1) + f1(2))>.count == 1);

// More functions than fit, or a name that does not fit: not known, so erased
static_assert(!needs<decltype(f1(1) + f2(1) + f3(1) + f4(1))>.partial);
static_assert(std::same_as<decltype(f1(1) + f2(1) + f3(1) + f4(1) + f5(1)), Expr>);
static_assert(std::same_as<decltype(too_long(1)), Expr>);


// Syntax ----------------------------------------------------------------------

static_assert(supports(anti, needs<decltype(col("a").ilike("x"))>));
static_assert(supports(anti, needs<decltype(col("a").any(std::vector<int>{}))>));
static_assert(supports(anti, needs<decltype(col("a").cast("int"))>));
static_assert(supports(anti, needs<decltype(now())>));
static_assert(supports(anti, only(ESyntax::Excluded)));

static_assert(!supports(nothing, needs<decltype(col("a").ilike("x"))>));
static_assert(!supports(nothing, needs<decltype(col("a").any(std::vector<int>{}))>));
static_assert(!supports(nothing, needs<decltype(col("a").cast("int"))>));
static_assert(!supports(nothing, needs<decltype(col("a").ilike("x") && col("b") == 1)>));
static_assert(!supports(nothing, only(ESyntax::Excluded)));
static_assert(supports(nothing, needs<decltype(col("a") == 1 && col("b").is_null())>));
static_assert(supports(nothing, needs<decltype(now())>));

static_assert(supports(picky, needs<decltype(greatest(col("a"), col("b")))>));
static_assert(!supports(picky, needs<decltype(greatest(col("a")))>));
static_assert(!supports(picky, needs<decltype(forbidden())>));
static_assert(!supports(picky, needs<decltype(coalesce(col("a"), forbidden(1)))>));
static_assert(supports(picky, needs<decltype(coalesce(col("a"), 1))>));

static_assert(missing(picky, needs<decltype(greatest(col("a")))>) == flux::impl::need_function("greatest", 1));

// Rules: a name with rules is refused for an argument count without one, a name without rules keeps its own
static_assert(supports(tabled, needs<decltype(pair(1, 2))>));
static_assert(!supports(tabled, needs<decltype(pair(1))>));
static_assert(!supports(tabled, needs<decltype(pair(1, 2, 3))>));
static_assert(supports(tabled, needs<decltype(greatest(col("a")))>));
static_assert(!supports(tabled_and_picky, needs<decltype(greatest(col("a")))>));
static_assert(!supports(tabled_and_picky, needs<decltype(pair(1))>));
static_assert(supports(tabled_and_picky, needs<decltype(pair(1, 2))>));

// The first rule wins
static_assert(flux::impl::spelling_of(mine_first, "pair", 2).name == "mine");
static_assert(flux::impl::spelling_of(mine_last, "pair", 2).name == "pair_of");
static_assert(flux::impl::spelling_of(mine_last, "pair", 5).name == "mine");
static_assert(supports(mine_first, needs<decltype(pair(1))>));
static_assert(missing(nothing, needs<decltype(col("a").ilike("x").cast("int"))>)
    == (only(ESyntax::Ilike) | only(ESyntax::Cast)));
static_assert(missing(anti, needs<decltype(col("a").ilike("x"))>) == no_needs);

// What is not known is not refused: the check at render time decides
static_assert(supports(nothing, flux::impl::unknown_needs));


// Rendering -------------------------------------------------------------------

FLUXMT_TEST_CASE("typed expr renders as the erased one", "[sql]") {
    const test::AntiDialect dialect;

    const auto typed = col("a").ilike("x%") && col("b") == 1;
    const Expr erased = typed;

    flux::sql::Context a(dialect);
    flux::sql::Context b(dialect);

    REQUIRE(typed.render(a));
    REQUIRE(erased.render(b));

    CHECK(a.text == "lower(a) LIKE lower(?1) AND b = ?2");
    CHECK(a.text == b.text);
}

FLUXMT_TEST_CASE("erased expr is still checked when it is rendered", "[sql]") {
    const BareDialect dialect;
    flux::sql::Context ctx(dialect);

    const Expr erased = col("a").cast("int");

    const auto r = erased.render(ctx);

    REQUIRE(!r);
    CHECK(r.error().code() == flux::EQueryError::Unsupported);
}

FLUXMT_TEST_CASE("typed and erased operands meet in an erased result", "[sql]") {
    const test::AntiDialect dialect;
    flux::sql::Context ctx(dialect);

    const Expr erased = col("b") == 1;
    const auto both = col("a").ilike("x") && erased;

    static_assert(std::same_as<std::remove_cvref_t<decltype(both)>, Expr>);

    REQUIRE(both.render(ctx));
    CHECK(ctx.text == "lower(a) LIKE lower(?1) AND b = ?2");
}
