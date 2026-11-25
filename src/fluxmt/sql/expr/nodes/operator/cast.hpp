#pragma once

// std
#include <string>
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/dialect.hpp>
#include <fluxmt/sql/error.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/node.hpp>
#include <fluxmt/sql/identifier.hpp>


namespace flux::impl::nodes {
    class CastNode final : public IExprNode {
    public:
        // Member functions

        CastNode(Expr operand, std::string type) :
            _operand(std::move(operand)),
            _type(std::move(type)) {
        }


        // Observers

        EPrecedence precedence() const noexcept override { return EPrecedence::Cast; }


        // Executors

        ers::Status render(Context& ctx) const override {
            if (!is_type_name(_type)) {
                return ers::make_error(ESqlError::InvalidName, "type '{}' in a cast",
                    _type);
            }

            if (ctx.dialect().caps().cast == ECastStyle::Postfix) {
                ERS_TRY(_operand.render(ctx, EPrecedence::Cast));

                ctx.query += "::";
                ctx.query += _type;

                return ers::ok;
            }

            ctx.query += "CAST(";
            ERS_TRY(_operand.render(ctx));

            ctx.query += " AS ";
            ctx.query += _type;
            ctx.query += ')';

            return ers::ok;
        }


    private:
        Expr _operand;
        std::string _type;
    };
}
