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


namespace flux::impl::nodes {
    class AnyNode final : public IExprNode {
    public:
        // Member functions

        AnyNode(Expr operand, Expr array) :
            _operand(std::move(operand)),
            _array(std::move(array)) {
        }


        // Observers

        EPrecedence precedence() const noexcept override { return EPrecedence::Comparison; }


        // Executors

        ers::Status render(SqlContext& ctx) const override {
            const auto any = ctx.dialect().syntax().any;

            if (!any) {
                return ers::make_error(EQueryError::Unsupported, "ANY in '{}'",
                    ctx.dialect().name());
            }

            return any(ctx, _operand, _array);
        }


    private:
        Expr _operand;
        Expr _array;
    };
}
