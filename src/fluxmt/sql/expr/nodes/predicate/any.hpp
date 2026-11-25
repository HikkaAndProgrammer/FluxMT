#pragma once

// std
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/core/value.hpp>
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/dialect.hpp>
#include <fluxmt/sql/error.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/node.hpp>

// nodes
#include "../terminal/bound.hpp"


namespace flux::impl::nodes {
    // Native arrays: "x = ANY(array)". Without them a bound array is spread into "x IN (...)".
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

        ers::Status render(Context& ctx) const override {
            if (ctx.dialect().caps().native_arrays) {
                ERS_TRY(_operand.render(ctx, EPrecedence::Like));

                ctx.query += " = ANY(";
                ERS_TRY(_array.render(ctx));

                ctx.query += ')';

                return ers::ok;
            }

            const auto* bound = dynamic_cast<const BoundNode*>(&_array.node());
            const auto* items = bound ? bound->value().get_if<value_array_t>() : nullptr;

            if (!items) {
                return ers::make_error(ESqlError::Unsupported, "ANY needs an array value, '{}' has none",
                    ctx.dialect().name());
            }

            if (items->empty()) {
                ctx.query += "FALSE";

                return ers::ok;
            }

            ERS_TRY(_operand.render(ctx, EPrecedence::Other));

            ctx.query += " IN (";

            bool first = true;
            for (const auto& it : *items) {
                if (!first)
                    ctx.query += ", ";
                first = false;

                ctx.query += ctx.bind(it);
            }

            ctx.query += ')';

            return ers::ok;
        }


    private:
        Expr _operand;
        Expr _array;
    };
}
