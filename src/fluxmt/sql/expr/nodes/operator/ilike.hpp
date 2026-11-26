#pragma once

// std
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/dialect.hpp>
#include <fluxmt/sql/error.hpp>
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

        ers::Status render(SqlContext& ctx) const override {
            const auto ilike = ctx.dialect().syntax().ilike;

            if (!ilike) {
                return ers::make_error(EQueryError::Unsupported, "ILIKE in '{}'",
                    ctx.dialect().name());
            }

            return ilike(ctx, _lhs, _rhs);
        }


    private:
        Expr _lhs;
        Expr _rhs;
    };
}
