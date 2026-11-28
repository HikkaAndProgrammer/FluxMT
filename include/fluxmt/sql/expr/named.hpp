#pragma once

// std
#include <string_view>
#include <utility>
#include <vector>

// ers
#include <erslib/core/type/fixed_string.hpp>

// fluxmt
#include <fluxmt/sql/expr/expr.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl {
    template<ers::fixed_string Op, EPrecedence Precedence = EPrecedence::Other>
    struct binary_op_fn {
        static_assert(!Op.to_sv().empty(), "binary_op_fn<> needs an operator.");

        template<IsExprLike L, IsExprLike R>
        auto operator()(L&& lhs, R&& rhs) const {
            return retype<needs_of<L, R>()>(
                make_binary(Op.to_sv(), Expr(std::forward<L>(lhs)), Expr(std::forward<R>(rhs)), Precedence));
        }
    };

    template<ers::fixed_string Name>
    struct named_fn {
        static_assert(!Name.to_sv().empty(), "named_fn<> needs a name.");

        template<typename... Args>
        auto operator()(Args&&... args) const {
            std::vector<Expr> items;
            items.reserve(sizeof...(Args));
            (items.push_back(Expr(std::forward<Args>(args))), ...);

            return retype<(needs_of<Args...>() | need_function(Name.to_sv(), sizeof...(Args)))>(
                make_fn(Name.to_sv(), std::move(items)));
        }
    };
}

namespace flux::impl::clauses {
    template<typename... Args>
    auto sql(sql_text pattern, Args&&... args) {
        std::vector<Expr> items;
        items.reserve(sizeof...(Args));
        (items.push_back(Expr(std::forward<Args>(args))), ...);

        return retype<needs_of<Args...>()>(make_raw(pattern, std::move(items)));
    }
}
