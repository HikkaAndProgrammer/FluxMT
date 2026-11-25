#pragma once

// std
#include <concepts>
#include <utility>

// fluxmt
#include <fluxmt/sql/expr/concepts.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/factories.hpp>


// Comparison ------------------------------------------------------------------

namespace flux::impl {
    template<typename L, typename R>
        requires IsExprOperands<L, R>
    Expr operator==(L&& lhs, R&& rhs) {
        return make_binary("=", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison);
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    Expr operator!=(L&& lhs, R&& rhs) {
        return make_binary("<>", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison);
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    Expr operator<(L&& lhs, R&& rhs) {
        return make_binary("<", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison);
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    Expr operator<=(L&& lhs, R&& rhs) {
        return make_binary("<=", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison);
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    Expr operator>(L&& lhs, R&& rhs) {
        return make_binary(">", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison);
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    Expr operator>=(L&& lhs, R&& rhs) {
        return make_binary(">=", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison);
    }
}


// Arithmetic ------------------------------------------------------------------

namespace flux::impl {
    template<typename L, typename R>
        requires IsExprOperands<L, R>
    Expr operator+(L&& lhs, R&& rhs) {
        return make_binary("+", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::AddSub, true);
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    Expr operator-(L&& lhs, R&& rhs) {
        return make_binary("-", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::AddSub, true);
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    Expr operator*(L&& lhs, R&& rhs) {
        return make_binary("*", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::MulDiv, true);
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    Expr operator/(L&& lhs, R&& rhs) {
        return make_binary("/", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::MulDiv, true);
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    Expr operator%(L&& lhs, R&& rhs) {
        return make_binary("%", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::MulDiv, true);
    }
}


// Logic -----------------------------------------------------------------------

namespace flux::impl {
    template<typename L, typename R>
        requires IsExprOperands<L, R>
    Expr operator&&(L&& lhs, R&& rhs) {
        return make_logical(true, Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)));
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    Expr operator||(L&& lhs, R&& rhs) {
        return make_logical(false, Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)));
    }

    // A template, so 'Result' and friends never reach it through Expr's converting constructor.
    template<std::same_as<Expr> T>
    Expr operator!(const T& what) {
        return make_not(what);
    }
}
