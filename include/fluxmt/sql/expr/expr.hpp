#pragma once

// std
#include <concepts>
#include <initializer_list>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/core/value_traits.hpp>
#include <fluxmt/sql/expr/concepts.hpp>
#include <fluxmt/sql/expr/factories.hpp>
#include <fluxmt/sql/expr/fwd.hpp>
#include <fluxmt/sql/expr/needs.hpp>
#include <fluxmt/sql/expr/node.hpp>

// export
#include <fluxmt/export.hpp>


// Details ---------------------------------------------------------------------

namespace flux::impl {
    template<needs_t N>
    class TypedExpr;

    template<needs_t N>
    using expr_for = std::conditional_t<N.partial, Expr, TypedExpr<N>>;

    template<needs_t N>
    expr_for<N> retype(Expr expr);

    template<typename... Ts>
    constexpr needs_t needs_of() noexcept;
}


// Expr ------------------------------------------------------------------------

namespace flux::impl {
    class FLUXMT_EXPORT Expr {
    public:
        static constexpr needs_t needs = unknown_needs;


        // Member functions

        explicit Expr(ExprNodePtr node);

        template<IsExprValue T>
        Expr(T&& value) :
            Expr(make_value(to_value(std::forward<T>(value)))) {
        }

        template<IsExprSource T>
        Expr(const T& source) :
            Expr(source.to_expr()) {
        }

        Expr(const Expr&) = default;
        Expr(Expr&&) noexcept = default;

        Expr& operator=(const Expr&) & = default;
        Expr& operator=(Expr&&) & noexcept = default;


        // Observers

        EPrecedence precedence() const noexcept;
        const IExprNode& node() const noexcept;


        // Generators

        template<typename Self>
        auto is_null(this const Self& self) {
            return retype<needs_of<Self>()>(
                make_postfix(self, " IS NULL", EPrecedence::Is));
        }

        template<typename Self>
        auto is_not_null(this const Self& self) {
            return retype<needs_of<Self>()>(
                make_postfix(self, " IS NOT NULL", EPrecedence::Is));
        }

        template<typename Self, IsExprLike P>
        auto is_distinct_from(this const Self& self, P&& other) {
            return retype<needs_of<Self, P>()>(
                make_binary("IS DISTINCT FROM", self, Expr(std::forward<P>(other)), EPrecedence::Is));
        }


        template<typename Self, IsExprLike P>
        auto like(this const Self& self, P&& pattern) {
            return retype<needs_of<Self, P>()>(
                make_binary("LIKE", self, Expr(std::forward<P>(pattern)), EPrecedence::Like));
        }

        template<typename Self, IsExprLike P>
        auto ilike(this const Self& self, P&& pattern) {
            return retype<(needs_of<Self, P>() | need_syntax(ESyntax::Ilike))>(
                make_ilike(self, Expr(std::forward<P>(pattern))));
        }

        template<typename Self, IsExprLike L, IsExprLike H>
        auto between(this const Self& self, L&& low, H&& high) {
            return retype<needs_of<Self, L, H>()>(
                make_between(self, Expr(std::forward<L>(low)), Expr(std::forward<H>(high))));
        }

        template<typename Self, IsExprLike P>
        auto contains(this const Self& self, P&& other) {
            return retype<needs_of<Self, P>()>(
                make_binary("@>", self, Expr(std::forward<P>(other)), EPrecedence::Other));
        }


        template<typename Self, IsExprLike T>
        auto in(this const Self& self, std::initializer_list<T> values) {
            std::vector<Expr> items;
            items.reserve(values.size());

            for (const auto& it : values)
                items.push_back(Expr(it));

            return retype<needs_of<Self, T>()>(
                make_in(self, std::move(items)));
        }

        template<typename Self>
        auto in(this const Self& self, std::initializer_list<Expr> values) {
            return retype<needs_of<Self, Expr>()>(
                make_in(self, std::vector(values)));
        }

        template<typename Self, std::ranges::input_range R>
        auto in(this const Self& self, R&& values) {
            std::vector<Expr> items;

            for (auto&& it : values)
                items.push_back(Expr(it));

            return retype<needs_of<Self, std::ranges::range_reference_t<R>>()>(
                make_in(self, std::move(items)));
        }

        template<typename Self, std::convertible_to<Query> Q>
        auto in(this const Self& self, Q subquery) {
            return retype<needs_of<Self, Expr>()>(
                make_in(self, Query(std::move(subquery))));
        }


        template<typename Self, IsExprLike P>
        auto any(this const Self& self, P&& array) {
            return retype<(needs_of<Self, P>() | need_syntax(ESyntax::Any))>(
                make_any(self, Expr(std::forward<P>(array))));
        }

        template<typename Self>
        auto cast(this const Self& self, std::string_view type) {
            return retype<(needs_of<Self>() | need_syntax(ESyntax::Cast))>(
                make_cast(self, type));
        }


        aliased_t as(std::string alias) const;


        // Executors

        ers::Status render(SqlContext& ctx, EPrecedence parent = EPrecedence::Or) const;


    private:
        ExprNodePtr _node;
    };
}


namespace flux::impl {
    template<needs_t N>
    class TypedExpr : public Expr {
        static_assert(!N.partial, "TypedExpr<> has to know all it needs, an erased expression is Expr.");


    public:
        static constexpr needs_t needs = N;


        explicit TypedExpr(Expr expr) noexcept :
            Expr(std::move(expr)) {
        }
    };


    template<needs_t N>
    expr_for<N> retype(Expr expr) {
        if constexpr (N.partial)
            return expr;
        else
            return expr_for<N>(std::move(expr));
    }

    template<typename T>
    constexpr needs_t needs_of_one() noexcept {
        using U = std::remove_cvref_t<T>;

        if constexpr (std::is_base_of_v<Expr, U>)
            return U::needs;
        else if constexpr (IsExprSource<U>)
            return needs_of_one<decltype(std::declval<const U&>().to_expr())>();
        else
            return no_needs;
    }

    template<typename... Ts>
    constexpr needs_t needs_of() noexcept {
        needs_t out;
        ((out = out | needs_of_one<Ts>()), ...);

        return out;
    }
}


namespace flux::impl {
    struct aliased_t {
        Expr expr;
        std::string alias;
    };

    inline aliased_t Expr::as(std::string alias) const {
        return { .expr = *this, .alias = std::move(alias) };
    }


    struct column_expr_t {
        Expr expr;


        template<IsExpr T>
        column_expr_t(T&& what) :
            expr(std::forward<T>(what)) {
        }

        template<typename S>
            requires (std::convertible_to<S, std::string_view> && !IsExpr<S>)
        column_expr_t(S&& name) :
            expr(make_column(std::string(std::string_view(name)))) {
        }
    };


    class ExprMethods {
    public:
        auto is_null(this const auto& self) {
            return self.to_expr().is_null();
        }
        auto is_not_null(this const auto& self) {
            return self.to_expr().is_not_null();
        }

        template<IsExprLike P>
        auto is_distinct_from(this const auto& self, P&& other) {
            return self.to_expr().is_distinct_from(std::forward<P>(other));
        }


        template<IsExprLike P>
        auto like(this const auto& self, P&& pattern) {
            return self.to_expr().like(std::forward<P>(pattern));
        }

        template<IsExprLike P>
        auto ilike(this const auto& self, P&& pattern) {
            return self.to_expr().ilike(std::forward<P>(pattern));
        }

        template<IsExprLike L, IsExprLike H>
        auto between(this const auto& self, L&& low, H&& high) {
            return self.to_expr().between(std::forward<L>(low), std::forward<H>(high));
        }

        template<IsExprLike P>
        auto contains(this const auto& self, P&& other) {
            return self.to_expr().contains(std::forward<P>(other));
        }


        template<IsExprLike T>
        auto in(this const auto& self, std::initializer_list<T> values) {
            return self.to_expr().in(values);
        }
        auto in(this const auto& self, std::initializer_list<Expr> values) {
            return self.to_expr().in(values);
        }
        template<std::ranges::input_range R>
        auto in(this const auto& self, R&& values) {
            return self.to_expr().in(std::forward<R>(values));
        }
        template<std::convertible_to<Query> Q>
        auto in(this const auto& self, Q subquery) {
            return self.to_expr().in(std::move(subquery));
        }


        template<IsExprLike P>
        auto any(this const auto& self, P&& array) {
            return self.to_expr().any(std::forward<P>(array));
        }

        auto cast(this const auto& self, std::string_view type) {
            return self.to_expr().cast(type);
        }

        aliased_t as(this const auto& self, std::string alias) {
            return self.to_expr().as(std::move(alias));
        }
    };
}


// Factories -------------------------------------------------------------------

namespace flux::impl {
    template<std::derived_from<IExprNode> T, typename... Args>
    Expr make_expr(Args&&... args) {
        return Expr(std::make_shared<const T>(std::forward<Args>(args)...));
    }


    namespace clauses {
        inline auto col(std::string name) {
            return retype<no_needs>(make_column(std::move(name)));
        }

        inline auto col(std::string_view table, std::string_view name) {
            return col(std::string(table) + '.' + std::string(name));
        }

        inline auto null() {
            return retype<no_needs>(make_const("NULL"));
        }

        inline auto sql_bool(bool value) {
            return retype<no_needs>(value ? make_const("TRUE") : make_const("FALSE"));
        }

        template<typename T>
        auto val(T&& value) {
            return retype<needs_of<T>()>(Expr(std::forward<T>(value)));
        }

        template<typename T>
        auto param(T&& value) {
            return retype<no_needs>(make_param(to_value(std::forward<T>(value))));
        }

        template<typename... Args>
        auto row(Args&&... items) {
            std::vector<Expr> out;
            out.reserve(sizeof...(Args));
            (out.push_back(Expr(std::forward<Args>(items))), ...);

            return retype<needs_of<Args...>()>(make_row(std::move(out)));
        }
    }
}
