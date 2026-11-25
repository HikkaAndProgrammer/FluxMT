#pragma once

// std
#include <concepts>
#include <type_traits>

// fluxmt
#include <fluxmt/sql/expr/fwd.hpp>


namespace flux::impl {
    template<typename T>
    concept IsExprSource = requires(const T& what) {
        { what.to_expr() } -> std::same_as<Expr>;
    };

    template<typename T>
    concept IsExprValue = !std::same_as<std::remove_cvref_t<T>, Expr>
        && !IsExprSource<std::remove_cvref_t<T>>
        && !std::convertible_to<T, ExprNodePtr>;

    template<typename T>
    concept IsExpr = std::same_as<std::remove_cvref_t<T>, Expr>
        || IsExprSource<std::remove_cvref_t<T>>;

    template<typename T>
    concept IsExprLike = IsExpr<T> || IsExprValue<T>;

    template<typename A, typename B>
    concept AcceptsOperand = !requires { std::remove_cvref_t<A>::template accepts<std::decay_t<B>>; }
        || std::remove_cvref_t<A>::template accepts<std::decay_t<B>>;

    template<typename L, typename R>
    concept IsExprOperands = IsExprLike<L> && IsExprLike<R> && (IsExpr<L> || IsExpr<R>)
        && AcceptsOperand<L, R> && AcceptsOperand<R, L>;
}
