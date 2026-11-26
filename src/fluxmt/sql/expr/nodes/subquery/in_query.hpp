#pragma once

// std
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/node.hpp>
#include <fluxmt/query/query.hpp>


namespace flux::impl::nodes {
    class InQueryNode final : public IExprNode {
    public:
        // Member functions

        InQueryNode(Expr operand, Query query) :
            _operand(std::move(operand)),
            _query(std::move(query)) {
        }


        // Observers

        EPrecedence precedence() const noexcept override { return EPrecedence::Like; }


        // Executors

        ers::Status render(SqlContext& ctx) const override {
            ERS_TRY(_operand.render(ctx, EPrecedence::Other));

            return append_subquery(ctx, _query, " IN (");
        }


    private:
        Expr _operand;
        Query _query;
    };
}
