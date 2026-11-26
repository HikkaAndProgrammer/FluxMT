#pragma once

// std
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/node.hpp>


namespace flux::impl::nodes {
    class NotNode final : public IExprNode {
    public:
        // Member functions

        explicit NotNode(Expr what) :
            _what(std::move(what)) {
        }


        // Observers

        EPrecedence precedence() const noexcept override { return EPrecedence::Not; }


        // Executors

        ers::Status render(SqlContext& ctx) const override {
            ctx.text += "NOT ";

            return _what.render(ctx, EPrecedence::Not);
        }


    private:
        Expr _what;
    };
}
