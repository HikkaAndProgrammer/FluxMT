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
    auto operator==(L&& lhs, R&& rhs) {
        return retype<needs_of<L, R>()>(
            make_binary("=", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison));
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    auto operator!=(L&& lhs, R&& rhs) {
        return retype<needs_of<L, R>()>(
            make_binary("<>", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison));
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    auto operator<(L&& lhs, R&& rhs) {
        return retype<needs_of<L, R>()>(
            make_binary("<", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison));
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    auto operator<=(L&& lhs, R&& rhs) {
        return retype<needs_of<L, R>()>(
            make_binary("<=", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison));
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    auto operator>(L&& lhs, R&& rhs) {
        return retype<needs_of<L, R>()>(
            make_binary(">", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison));
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    auto operator>=(L&& lhs, R&& rhs) {
        return retype<needs_of<L, R>()>(
            make_binary(">=", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::Comparison));
    }
}


// Arithmetic ------------------------------------------------------------------

namespace flux::impl {
    template<typename L, typename R>
        requires IsExprOperands<L, R>
    auto operator+(L&& lhs, R&& rhs) {
        return retype<needs_of<L, R>()>(
            make_binary("+", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::AddSub, true));
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    auto operator-(L&& lhs, R&& rhs) {
        return retype<needs_of<L, R>()>(
            make_binary("-", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::AddSub, true));
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    auto operator*(L&& lhs, R&& rhs) {
        return retype<needs_of<L, R>()>(
            make_binary("*", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::MulDiv, true));
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    auto operator/(L&& lhs, R&& rhs) {
        return retype<needs_of<L, R>()>(
            make_binary("/", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::MulDiv, true));
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    auto operator%(L&& lhs, R&& rhs) {
        return retype<needs_of<L, R>()>(
            make_binary("%", Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), EPrecedence::MulDiv, true));
    }
}


// Logic -----------------------------------------------------------------------

namespace flux::impl {
    template<typename L, typename R>
        requires IsExprOperands<L, R>
    auto operator&&(L&& lhs, R&& rhs) {
        return retype<needs_of<L, R>()>(
            make_logical(true, Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs))));
    }

    template<typename L, typename R>
        requires IsExprOperands<L, R>
    auto operator||(L&& lhs, R&& rhs) {
        return retype<needs_of<L, R>()>(
            make_logical(false, Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs))));
    }

    // A template, so 'Result' and friends never reach it through Expr's converting constructor.
    template<std::derived_from<Expr> T>
    auto operator!(const T& what) {
        return retype<needs_of<T>()>(make_not(what));
    }
}
