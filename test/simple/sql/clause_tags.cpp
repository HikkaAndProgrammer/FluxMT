// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <concepts>
#include <vector>

// fluxmt
#include <fluxmt/sql.hpp>

// test
#include "support/test_case.hpp"


using namespace flux::orm;

using flux::sql::need_syntax;
using flux::sql::clause_needs;
using flux::sql::ESyntax;
using flux::sql::no_needs;


FLUXMT_TEST_CASE("clause factories return tags of their slot", "[sql]") {
    STATIC_REQUIRE(decltype(where(col("a") == 1))::slot == &flux::slots::where);
    STATIC_REQUIRE(decltype(having(col("a") == 1))::slot == &flux::slots::having);
    STATIC_REQUIRE(decltype(join("t", col("a") == col("b")))::slot == &flux::slots::join);
    STATIC_REQUIRE(decltype(from("t"))::slot == &flux::slots::from);
    STATIC_REQUIRE(decltype(distinct())::slot == &flux::slots::distinct);
    STATIC_REQUIRE(decltype(order_by("a"))::slot == &flux::slots::order);
    STATIC_REQUIRE(decltype(with_limit(1))::slot == &flux::slots::limit);
    STATIC_REQUIRE(decltype(with_offset(1))::slot == &flux::slots::offset);
    STATIC_REQUIRE(decltype(returning("id"))::slot == &flux::slots::returning);
    STATIC_REQUIRE(decltype(returning_all())::slot == &flux::slots::returning);
    STATIC_REQUIRE(decltype(columns("id"))::slot == &flux::slots::column);
    STATIC_REQUIRE(decltype(group_by("a"))::slot == &flux::slots::group);
    STATIC_REQUIRE(decltype(set("a", 1))::slot == &flux::slots::set);
    STATIC_REQUIRE(decltype(values(1, 2))::slot == &flux::slots::values);
    STATIC_REQUIRE(decltype(on_conflict("id"))::slot == &flux::slots::conflict);
    STATIC_REQUIRE(decltype(on_conflict_constraint("c"))::slot == &flux::slots::conflict_constraint);
    STATIC_REQUIRE(decltype(do_nothing())::slot == &flux::slots::conflict_action);
    STATIC_REQUIRE(decltype(do_update("a", "b"))::slot == &flux::slots::conflict_action);
}

FLUXMT_TEST_CASE("clause tags know how many clauses they hold", "[sql]") {
    STATIC_REQUIRE(!decltype(where(col("a") == 1))::many);
    STATIC_REQUIRE(decltype(returning("id"))::many);

    STATIC_REQUIRE(std::convertible_to<decltype(where(col("a") == 1)), flux::query::Clause>);
    STATIC_REQUIRE(!std::convertible_to<decltype(returning("id")), flux::query::Clause>);
    STATIC_REQUIRE(std::convertible_to<decltype(returning("id")), std::vector<flux::query::Clause>>);
}

FLUXMT_TEST_CASE("clause tags carry what the expressions need", "[sql]") {
    STATIC_REQUIRE(clause_needs<decltype(where(col("a") == 1))>() == no_needs);
    STATIC_REQUIRE(clause_needs<decltype(where(col("a").ilike("x%")))>() == need_syntax(ESyntax::Ilike));
    STATIC_REQUIRE(clause_needs<decltype(having(col("a").ilike("x%")))>() == need_syntax(ESyntax::Ilike));
    STATIC_REQUIRE(clause_needs<decltype(do_update("a", "b"))>() == need_syntax(ESyntax::Excluded));
    STATIC_REQUIRE(clause_needs<decltype(do_nothing())>() == no_needs);
    STATIC_REQUIRE(clause_needs<decltype(returning("id", "name"))>() == no_needs);

    STATIC_REQUIRE(clause_needs<decltype(where(flux::sql::Expr(col("a") == 1)))>().partial);
    STATIC_REQUIRE(clause_needs<decltype(columns(col("a").as("b")))>().partial);
}

FLUXMT_TEST_CASE("when keeps the slot and the needs of its clause", "[sql]") {
    using conditional = decltype(when(true, where(col("a").ilike("x%"))));

    STATIC_REQUIRE(conditional::slot == &flux::slots::where);
    STATIC_REQUIRE(conditional::many);
    STATIC_REQUIRE(clause_needs<conditional>() == need_syntax(ESyntax::Ilike));

    using lazy = decltype(when(true, [] { return where(col("a") == 1); }));

    STATIC_REQUIRE(lazy::slot == &flux::slots::where);
    STATIC_REQUIRE(clause_needs<lazy>() == no_needs);
}
