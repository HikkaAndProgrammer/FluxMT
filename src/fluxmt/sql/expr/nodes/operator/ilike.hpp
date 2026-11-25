#pragma once

// std
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/dialect.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/node.hpp>

// nodes
#include "../common.hpp"


namespace flux::impl::nodes {
    class IlikeNode final : public IExprNode {
    public:
        // Member functions

        IlikeNode(Expr lhs, Expr rhs) :
            _lhs(std::move(lhs)),
            _rhs(std::move(rhs)) {
        }


        // Observers

        EPrecedence precedence() const noexcept override { return EPrecedence::Like; }


        // Executors

        ers::Status render(Context& ctx) const override {
            if (ctx.dialect().caps().ilike) {
                ERS_TRY(_lhs.render(ctx, next(precedence())));

                ctx.query += " ILIKE ";

                return _rhs.render(ctx, next(precedence()));
            }

            ctx.query += "lower(";
            ERS_TRY(_lhs.render(ctx));

            ctx.query += ") LIKE lower(";
            ERS_TRY(_rhs.render(ctx));

            ctx.query += ')';

            return ers::ok;
        }


    private:
        Expr _lhs;
        Expr _rhs;
    };
}
