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
    class BetweenNode final : public IExprNode {
    public:
        // Member functions

        BetweenNode(Expr operand, Expr low, Expr high) :
            _operand(std::move(operand)),
            _low(std::move(low)),
            _high(std::move(high)) {
        }


        // Observers

        EPrecedence precedence() const noexcept override { return EPrecedence::Like; }


        // Executors

        ers::Status render(Context& ctx) const override {
            ERS_TRY(_operand.render(ctx, EPrecedence::Other));

            ctx.query += " BETWEEN ";
            ERS_TRY(_low.render(ctx, EPrecedence::Other));

            ctx.query += " AND ";

            return _high.render(ctx, EPrecedence::Other);
        }


    private:
        Expr _operand;
        Expr _low;
        Expr _high;
    };
}
