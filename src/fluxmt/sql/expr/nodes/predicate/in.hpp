#pragma once

// std
#include <utility>
#include <vector>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/node.hpp>

// nodes
#include "../common.hpp"


namespace flux::impl::nodes {
    class InNode final : public IExprNode {
    public:
        // Member functions

        InNode(Expr operand, std::vector<Expr> items) :
            _operand(std::move(operand)),
            _items(std::move(items)) {
        }


        // Observers

        EPrecedence precedence() const noexcept override {
            return _items.empty() ? EPrecedence::Primary : EPrecedence::Like;
        }


        // Executors

        ers::Status render(Context& ctx) const override {
            if (_items.empty()) {
                ctx.query += "FALSE";

                return ers::ok;
            }

            ERS_TRY(_operand.render(ctx, EPrecedence::Other));

            ctx.query += " IN (";
            ERS_TRY(render_list(ctx, _items));

            ctx.query += ')';

            return ers::ok;
        }


    private:
        Expr _operand;
        std::vector<Expr> _items;
    };
}
