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
    class RowNode final : public IExprNode {
    public:
        // Member functions

        explicit RowNode(std::vector<Expr> items) :
            _items(std::move(items)) {
        }


        // Executors

        ers::Status render(SqlContext& ctx) const override {
            ctx.text += '(';
            ERS_TRY(render_list(ctx, _items));

            ctx.text += ')';

            return ers::ok;
        }


    private:
        std::vector<Expr> _items;
    };
}
