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


namespace flux::impl::nodes {
    class LogicalNode final : public IExprNode {
    public:
        // Member functions

        LogicalNode(bool conjunction, std::vector<Expr> items) :
            _conjunction(conjunction),
            _items(std::move(items)) {
        }


        // Observers

        EPrecedence precedence() const noexcept override {
            return _conjunction ? EPrecedence::And : EPrecedence::Or;
        }

        bool conjunction() const noexcept { return _conjunction; }
        const std::vector<Expr>& items() const noexcept { return _items; }


        // Executors

        ers::Status render(SqlContext& ctx) const override {
            bool first = true;

            for (const auto& it : _items) {
                if (!first)
                    ctx.text += _conjunction ? " AND " : " OR ";
                first = false;

                ERS_TRY(it.render(ctx, precedence()));
            }

            return ers::ok;
        }


    private:
        bool _conjunction;
        std::vector<Expr> _items;
    };
}
