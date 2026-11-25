#pragma once

// std
#include <concepts>
#include <initializer_list>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/core/value_traits.hpp>
#include <fluxmt/sql/expr/concepts.hpp>
#include <fluxmt/sql/expr/factories.hpp>
#include <fluxmt/sql/expr/fwd.hpp>
#include <fluxmt/sql/expr/node.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    class FLUXMT_EXPORT Expr {
    public:
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

        Expr is_null() const;

        Expr is_not_null() const;

        Expr is_distinct_from(Expr other) const;


        Expr like(Expr pattern) const;

        Expr ilike(Expr pattern) const;

        Expr between(Expr low, Expr high) const;

        Expr contains(Expr other) const;


        Expr in(std::initializer_list<Expr> values) const;

        template<std::ranges::input_range R>
        Expr in(R&& values) const {
            std::vector<Expr> items;

            for (auto&& it : values)
                items.push_back(Expr(it));

            return make_in(*this, std::move(items));
        }


        Expr any(Expr array) const;

        Expr cast(std::string_view type) const;


        // For columns() and returning(): "<expr> AS <alias>"
        Aliased as(std::string alias) const;


        // Executors

        ers::Status render(Context& ctx, EPrecedence parent = EPrecedence::Or) const;


    private:
        ExprNodePtr _node;
    };
}


namespace flux::impl {
    struct Aliased {
        Expr expr;
        std::string alias;
    };

    inline Aliased Expr::as(std::string alias) const {
        return { .expr = *this, .alias = std::move(alias) };
    }


    // A column name or an expression, where a bare name means a column.
    struct ColumnExpr {
        Expr expr;


        template<IsExpr T>
        ColumnExpr(T&& what) :
            expr(std::forward<T>(what)) {
        }

        template<typename S>
            requires (std::convertible_to<S, std::string_view> && !IsExpr<S>)
        ColumnExpr(S&& name) :
            expr(clauses::col(std::string(std::string_view(name)))) {
        }
    };


    // The same builder methods for anything with 'Expr to_expr() const': derive from ExprMethods.
    class ExprMethods {
    public:
        Expr is_null(this const auto& self) { return self.to_expr().is_null(); }
        Expr is_not_null(this const auto& self) { return self.to_expr().is_not_null(); }
        Expr is_distinct_from(this const auto& self, Expr other) { return self.to_expr().is_distinct_from(std::move(other)); }


        Expr like(this const auto& self, Expr pattern) { return self.to_expr().like(std::move(pattern)); }
        Expr ilike(this const auto& self, Expr pattern) { return self.to_expr().ilike(std::move(pattern)); }
        Expr between(this const auto& self, Expr low, Expr high) { return self.to_expr().between(std::move(low), std::move(high)); }
        Expr contains(this const auto& self, Expr other) { return self.to_expr().contains(std::move(other)); }


        Expr in(this const auto& self, std::initializer_list<Expr> values) { return self.to_expr().in(values); }

        template<std::ranges::input_range R>
        Expr in(this const auto& self, R&& values) { return self.to_expr().in(std::forward<R>(values)); }


        Expr any(this const auto& self, Expr array) { return self.to_expr().any(std::move(array)); }
        Expr cast(this const auto& self, std::string_view type) { return self.to_expr().cast(type); }

        Aliased as(this const auto& self, std::string alias) { return self.to_expr().as(std::move(alias)); }
    };
}


// Factories -------------------------------------------------------------------

namespace flux::impl {
    template<std::derived_from<IExprNode> T, typename... Args>
    Expr make_expr(Args&&... args) {
        return Expr(std::make_shared<const T>(std::forward<Args>(args)...));
    }


    namespace clauses {
        template<typename T>
        Expr val(T&& value) {
            return Expr(std::forward<T>(value));
        }

        // One placeholder per statement, however many times it's used.
        template<typename T>
        Expr param(T&& value) {
            return make_param(to_value(std::forward<T>(value)));
        }

        template<typename... Args>
        Expr row(Args&&... items) {
            std::vector<Expr> out;
            out.reserve(sizeof...(Args));
            (out.push_back(Expr(std::forward<Args>(items))), ...);

            return make_row(std::move(out));
        }
    }
}
