#pragma once

// std
#include <string>
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
    class BinaryNode final : public IExprNode {
    public:
        // Member functions

        BinaryNode(std::string op, Expr lhs, Expr rhs, EPrecedence precedence, bool left_associative) :
            _op(std::move(op)),
            _lhs(std::move(lhs)),
            _rhs(std::move(rhs)),
            _precedence(precedence),
            _left_associative(left_associative) {
        }


        // Observers

        EPrecedence precedence() const noexcept override { return _precedence; }


        // Executors

        ers::Status render(SqlContext& ctx) const override {
            ERS_TRY(_lhs.render(ctx, _left_associative ? _precedence : next(_precedence)));

            ctx.text += ' ';
            ctx.text += _op;
            ctx.text += ' ';

            return _rhs.render(ctx, next(_precedence));
        }


    private:
        std::string _op;
        Expr _lhs;
        Expr _rhs;
        EPrecedence _precedence;
        bool _left_associative;
    };
}
