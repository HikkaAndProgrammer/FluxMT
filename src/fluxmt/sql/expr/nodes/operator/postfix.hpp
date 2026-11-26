#pragma once

// std
#include <string_view>
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/node.hpp>

// nodes
#include "../common.hpp"


namespace flux::impl::nodes {
    class PostfixNode final : public IExprNode {
    public:
        // Member functions

        PostfixNode(Expr operand, std::string_view suffix, EPrecedence precedence) :
            _operand(std::move(operand)),
            _suffix(suffix),
            _precedence(precedence) {
        }


        // Observers

        EPrecedence precedence() const noexcept override { return _precedence; }


        // Executors

        ers::Status render(SqlContext& ctx) const override {
            ERS_TRY(_operand.render(ctx, next(_precedence)));

            ctx.text += _suffix;

            return ers::ok;
        }


    private:
        Expr _operand;
        std::string_view _suffix;
        EPrecedence _precedence;
    };
}
