#pragma once

// std
#include <string_view>
#include <utility>
#include <vector>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/dialect.hpp>
#include <fluxmt/sql/error.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/node.hpp>
#include <fluxmt/sql/identifier.hpp>
#include <fluxmt/sql/syntax.hpp>

// nodes
#include "../common.hpp"


namespace flux::impl::nodes {
    class FnNode final : public IExprNode {
    public:
        // Member functions

        FnNode(std::string_view name, std::vector<Expr> args) :
            _name(name),
            _args(std::move(args)) {
        }


        // Executors

        ers::Status render(SqlContext& ctx) const override {
            if (!is_valid_name(_name)) {
                return ers::make_error(ESqlError::InvalidName, "function '{}'",
                    _name);
            }

            const auto spelling = spelling_of(ctx.dialect().syntax(), _name, _args.size());

            if (!spelling.supported) {
                return ers::make_error(EQueryError::Unsupported, "{}() in '{}': {}",
                    _name, ctx.dialect().name(), spelling.reason);
            }

            ctx.text += spelling.text.empty() ? _name : spelling.text;

            if (!spelling.call)
                return ers::ok;

            ctx.text += '(';
            ERS_TRY(render_list(ctx, _args));

            ctx.text += ')';

            return ers::ok;
        }


    private:
        std::string_view _name;
        std::vector<Expr> _args;
    };
}
